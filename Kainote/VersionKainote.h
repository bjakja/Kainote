//  Copyright (c) 2012 - 2026, Marcin Drob

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

//  Kainote's version, a semantic version (https://semver.org): bump MAJOR for
//  changes that break existing configs, scripts or workflows, MINOR for new
//  features, PATCH for fixes only.  A release candidate takes a prerelease
//  suffix, "1.2.0-rc.1", "1.2.0-rc.2", then "1.2.0" for the release itself.
//  Pushing the tag "v" + VersionKainote publishes a GitHub release, marked as
//  a prerelease when the version has a suffix.
//
//  NumVersionKainote is MAJOR,MINOR,PATCH,0 for the Windows version resource;
//  UpdateChecker.cpp fails the build if the two disagree.
//
//  The exact build is identified by gitparams.h, which is generated and
//  gitignored.

#define VersionKainote "1.1.0"
#define NumVersionKainote 1,1,0,0
