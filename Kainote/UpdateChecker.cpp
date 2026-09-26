//  Copyright (c) 2018 - 2026, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

#include "UpdateChecker.h"
#include "JsonValue.h"
#include "KaiCheckBox.h"
#include "KaiDialog.h"
#include "KaiMessageBox.h"
#include "KaiStaticText.h"
#include "KaiTextCtrl.h"
#include "MappedButton.h"
#include "SemVer.h"
#include "VersionKainote.h"
#include "config.h"
#include <wx/sizer.h>
#include <wx/utils.h>
#include <wx/webrequest.h>
#include <ctime>

namespace
{
	constexpr std::optional<SemVer> kVersion = ParseSemVer(VersionKainote);
	constexpr long long kNumVersion[] = { NumVersionKainote };
	static_assert(kVersion, "VersionKainote is not a semantic version");
	static_assert(kNumVersion[0] == kVersion->major && kNumVersion[1] == kVersion->minor &&
		kNumVersion[2] == kVersion->patch && kNumVersion[3] == 0,
		"NumVersionKainote does not match VersionKainote");

	// One place for a fork to change.
	const wxString kReleasesUrl =
		L"https://api.github.com/repos/bjakja/Kainote/releases?per_page=10";

	constexpr time_t kDay  = 24 * 60 * 60;
	constexpr time_t kWeek = 7 * kDay;

	struct ReleaseInfo
	{
		wxString tag;
		wxString name;
		wxString url;
		wxString notes;
	};

	// The list endpoint is used rather than /releases/latest because latest
	// hides prereleases outright, which would leave the "stable only" option
	// with nothing to do.
	bool FindNewerRelease(const wxString &body, bool stableOnly, ReleaseInfo *out)
	{
		JsonValue root = JsonValue::Parse(std::string(body.utf8_str()));
		if (root.GetType() != JsonValue::Type::Array)
			return false;

		for (const JsonValue &release : root.Items()) {
			if (release.GetBool("draft"))
				continue;
			wxString tag = release.GetString("tag_name");
			if (stableOnly && (release.GetBool("prerelease") || UpdateChecker::IsPrerelease(tag)))
				continue;
			if (tag.empty() || !UpdateChecker::IsNewerVersion(tag, VersionKainote))
				continue;

			out->tag = tag;
			out->name = release.GetString("name", tag);
			out->url = release.GetString("html_url");
			out->notes = release.GetString("body");
			return true;
		}
		return false;
	}

	class UpdateAvailableDialog : public KaiDialog
	{
	public:
		UpdateAvailableDialog(wxWindow *parent, const ReleaseInfo &release)
			: KaiDialog(parent, -1, _("A new version is available"))
		{
			wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

			KaiStaticText *title = new KaiStaticText(this, -1, release.name);
			title->SetFont(title->GetFont().Bold());
			sizer->Add(title, 0, wxEXPAND | wxALL, 4);

			sizer->Add(new KaiStaticText(this, -1,
				wxString::Format(_("You have version %s; version %s is available"),
					VersionKainote, release.tag)),
				0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

			KaiTextCtrl *notes = new KaiTextCtrl(this, -1, release.notes,
				wxDefaultPosition, wxSize(460, 220),
				wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP);
			sizer->Add(notes, 1, wxEXPAND | wxALL, 4);

			sizer->Add(new KaiStaticText(this, -1, release.url),
				0, wxEXPAND | wxLEFT | wxRIGHT, 4);

			autoCheck = new KaiCheckBox(this, -1, _("Check for updates automatically"));
			autoCheck->SetValue(Options.GetBool(UPDATER_AUTO_CHECK));
			sizer->Add(autoCheck, 0, wxEXPAND | wxALL, 4);

			stableOnly = new KaiCheckBox(this, -1, _("Stable versions only"));
			stableOnly->SetValue(Options.GetBool(UPDATER_CHECK_FOR_STABLE));
			sizer->Add(stableOnly, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

			wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
			// A MappedButton rather than wxHyperlinkCtrl: nothing else in the
			// UI is a stock control, so a stock one ignores the theme.
			MappedButton *open = new MappedButton(this, 20001, _("Open the download page"));
			MappedButton *later = new MappedButton(this, 20002, _("Remind me in a week"));
			MappedButton *close = new MappedButton(this, wxID_CLOSE, _("Close"));
			buttons->Add(open, 1, wxALL, 2);
			buttons->Add(later, 1, wxALL, 2);
			buttons->Add(close, 1, wxALL, 2);
			sizer->Add(buttons, 0, wxEXPAND | wxALL, 2);

			url = release.url;
			Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &) {
				if (!url.empty())
					wxLaunchDefaultBrowser(url);
			}, 20001);
			Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &) {
				Options.SetInt(UPDATER_NEXT_CHECK, (int)(time(nullptr) + kWeek));
				EndModal(wxID_CLOSE);
			}, 20002);
			Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent &) {
				EndModal(wxID_CLOSE);
			}, wxID_CLOSE);

			SetEscapeId(wxID_CLOSE);
			SetSizerAndFit(sizer);
		}

		void SaveChoices()
		{
			Options.SetBool(UPDATER_AUTO_CHECK, autoCheck->GetValue());
			Options.SetBool(UPDATER_CHECK_FOR_STABLE, stableOnly->GetValue());
			Options.SaveOptions(true, false);
		}

	private:
		KaiCheckBox *autoCheck;
		KaiCheckBox *stableOnly;
		wxString url;
	};

	void Check(wxWindow *parent, bool interactive)
	{
		wxWebRequest request = wxWebSession::GetDefault().CreateRequest(parent, kReleasesUrl);
		if (!request.IsOk()) {
			if (interactive)
				KaiMessageBox(_("Cannot check for updates"), _("Update"));
			return;
		}

		request.SetHeader(L"Accept", L"application/vnd.github+json");
		request.SetHeader(L"X-GitHub-Api-Version", L"2022-11-28");
		request.SetHeader(L"User-Agent", wxString::Format(L"Kainote/%s", VersionKainote));

		parent->Bind(wxEVT_WEBREQUEST_STATE, [parent, interactive](wxWebRequestEvent &evt) {
			if (evt.GetState() == wxWebRequest::State_Active ||
				evt.GetState() == wxWebRequest::State_Idle)
			{
				return;
			}

			// An automatic check backs off a day whatever happened, so a
			// server that is down does not mean a request on every start.
			Options.SetInt(UPDATER_NEXT_CHECK, (int)(time(nullptr) + kDay));

			if (evt.GetState() != wxWebRequest::State_Completed) {
				if (interactive)
					KaiMessageBox(_("Cannot check for updates"), _("Update"));
				return;
			}

			ReleaseInfo release;
			if (!FindNewerRelease(evt.GetResponse().AsString(),
					Options.GetBool(UPDATER_CHECK_FOR_STABLE), &release))
			{
				if (interactive)
					KaiMessageBox(_("You already have the latest version"), _("Update"));
				return;
			}

			UpdateAvailableDialog dialog(parent, release);
			dialog.ShowModal();
			dialog.SaveChoices();
		});

		request.Start();
	}
}

bool UpdateChecker::IsNewerVersion(const wxString &tag, const wxString &current)
{
	std::string left = tag.ToStdString();
	std::string right = current.ToStdString();
	std::optional<SemVer> newer = ParseSemVer(left);
	std::optional<SemVer> installed = ParseSemVer(right);
	return newer && installed && CompareSemVer(*newer, *installed) > 0;
}

bool UpdateChecker::IsPrerelease(const wxString &tag)
{
	std::string text = tag.ToStdString();
	std::optional<SemVer> version = ParseSemVer(text);
	return version && !version->prerelease.empty();
}

void UpdateChecker::CheckOnStartup(wxWindow *parent)
{
	if (!Options.GetBool(UPDATER_AUTO_CHECK))
		return;
	if (time(nullptr) < (time_t)Options.GetInt(UPDATER_NEXT_CHECK))
		return;

	Check(parent, false);
}

void UpdateChecker::CheckNow(wxWindow *parent)
{
	Check(parent, true);
}
