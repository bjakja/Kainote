//  Copyright (c) 2017, £ukasz G¹sowski

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

#include "LanguageTool.h"
#include "windows.h"
#include <string.h>


LanguageTool* LanguageTool::instance = nullptr;
bool LanguageTool::init(){                     // Pointer to native interface
	wchar_t path[_MAX_PATH];
	wchar_t * pch;
	/*HMODULE phModule = NULL;
	wchar_t dummyChar;

	if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCWSTR)&dummyChar,
		&phModule))
	{
		cerr << "Cannot get handlename";
	}*/
	GetModuleFileNameW(NULL, path, sizeof(path));
	wxString KainotePath = wxString(path).BeforeLast('\\');
	KainotePath.Replace("\\", "/");
	KainotePath.Prepend("-Djava.class.path=");
	KainotePath += "/LanguageTool/languagetool-facade-4.0-jar-with-dependencies.jar";
       //================== prepare loading of Java VM ============================
    JavaVMInitArgs vm_args;                        // Initialization arguments
    JavaVMOption* options = new JavaVMOption[4];   // JVM invocation options
    //options[0].optionString = "-Djava.class.path=languagetool-master/languagetool-core/target/classes";   // where to find java .class

	strcpy(options[0].optionString, KainotePath.mb_str(wxConvUTF8).data());   // where to find java .class
    //options[1].optionString = "-verbose:jni";
    vm_args.version = JNI_VERSION_1_6;             // minimum Java version
    vm_args.nOptions = 1;                          // number of options
    vm_args.options = options;
    vm_args.ignoreUnrecognized = false;     // invalid options make the JVM init fail
       //=============== load and initialize Java VM and JNI interface =============
    jint rc = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);  // YES !!
    delete options;    // we then no longer need the initialisation options.
    if (rc != JNI_OK) {
          cerr << "ERROR: JNI_OK not ok!";
         return false;
    }
      //=============== Display JVM version =======================================

    class_LanguageToolFacade = env->FindClass("org/languagetool/facade/LanguageToolFacade");  // try to find the class
    if(class_LanguageToolFacade == nullptr) {
        if( env->ExceptionOccurred() )
            env->ExceptionDescribe();
        else
            cerr << "ERROR: class class_LanguageToolFacade not found but no exceptions!";
            return false;
    }
    class_RuleMatch = env->FindClass("org/languagetool/rules/RuleMatch");
    if(class_RuleMatch == nullptr) {
        if( env->ExceptionOccurred() )
            env->ExceptionDescribe();
        else
            cerr << "ERROR: class class_RuleMatch not found but no exceptions!";
            return false;
    }

    RuleMatch_getFromPos = env->GetMethodID(class_RuleMatch,"getFromPos","()I");
    if(RuleMatch_getFromPos == nullptr) {
        if( env->ExceptionOccurred() )
            env->ExceptionDescribe();
        else
            cerr << "ERROR: method RuleMatch_getFromPos not found but no exceptions!";
            return false;
    }

    RuleMatch_getToPos = env->GetMethodID(class_RuleMatch,"getToPos","()I");
    if(RuleMatch_getFromPos == nullptr) {
        if( env->ExceptionOccurred() )
            env->ExceptionDescribe();
        else
            cerr << "ERROR: method RuleMatch_getFromPos not found but no exceptions!";
            return false;
    }

    RuleMatch_message = env->GetFieldID(class_RuleMatch,"message","Ljava/lang/String;");
    if(RuleMatch_message == nullptr) {
        if( env->ExceptionOccurred() )
            env->ExceptionDescribe();
        else
            cerr << "ERROR: field RuleMatch_message not found but no exceptions!";
            return false;
    }

    RuleMatch_replacements = env->GetFieldID(class_RuleMatch,"suggestedReplacements","Ljava/util/List;");
    if(RuleMatch_message == nullptr) {
        if( env->ExceptionOccurred() )
            env->ExceptionDescribe();
        else
            cerr << "ERROR: field RuleMatch_replacements not found but no exceptions!";
            return false;
    }

    class_List = env->FindClass("java/util/List");
    if(class_List == nullptr) {
        if( env->ExceptionOccurred() )
            env->ExceptionDescribe();
        else
            cerr << "ERROR: class class_List not found but no exceptions!";
            return false;
    }

    List_toArray = env->GetMethodID(class_List,"toArray","()[Ljava/lang/Object;");
    if(List_toArray == nullptr) {
        if( env->ExceptionOccurred() )
            env->ExceptionDescribe();
        else
            cerr << "ERROR: method List_toArray not found but no exceptions!";
            return false;
    }


    LanguageToolFacade_checkText =  env->GetStaticMethodID(class_LanguageToolFacade, "checkText", "(Ljava/lang/String;)[Lorg/languagetool/rules/RuleMatch;");
    if(LanguageToolFacade_checkText == nullptr) {
        if( env->ExceptionOccurred() )
            env->ExceptionDescribe();
        else
            cerr << "ERROR: method LanguageToolFacade_checkText not found but no exceptions!";
            return false;
    }
	LanguageToolFacade_setLanguage = env->GetStaticMethodID(class_LanguageToolFacade, "setLanguage", "(Ljava/lang/String;)Z");
	if (LanguageToolFacade_setLanguage == nullptr) {
		if (env->ExceptionOccurred())
			env->ExceptionDescribe();
		else
			cerr << "ERROR: method LanguageToolFacade_setLanguage not found but no exceptions!";
		return false;
	}

	LanguageToolFacade_getLanguages = env->GetStaticMethodID(class_LanguageToolFacade, "getLanguages", "()[Ljava/lang/String;");
	if (LanguageToolFacade_getLanguages == nullptr) {
		if (env->ExceptionOccurred())
			env->ExceptionDescribe();
		else
			cerr << "ERROR: method LanguageToolFacade_getLanguages not found but no exceptions!";
		return false;
	}

    return true;

}
void LanguageTool::checkText(const wxString &text_to_check, vector<RuleMatch> &errors)
{
    jstring text = env->NewStringUTF(text_to_check.c_str());
    cout << "Start" << endl;
    jobjectArray result = (jobjectArray)env->CallObjectMethod(class_LanguageToolFacade, LanguageToolFacade_checkText,text);
    cout << "End" << endl;
    jsize len = env->GetArrayLength(result);
    for(int i=0;i<len;i++){
        jobject rule = env->GetObjectArrayElement(result, (jsize)i);
        jint a = env->CallIntMethod(rule,RuleMatch_getFromPos);
        jint b = env->CallIntMethod(rule,RuleMatch_getToPos);
        jstring s = (jstring) env->GetObjectField( rule, RuleMatch_message);
        jobjectArray r = (jobjectArray) env->CallObjectMethod(env->GetObjectField( rule, RuleMatch_replacements), List_toArray);
        jsize r_len = env->GetArrayLength(r);
        errors.push_back(RuleMatch(env->GetStringUTFChars(s,0), a, b));

        for(int j=0; j< r_len;j++){
            jstring s = (jstring) env->GetObjectArrayElement(r, j);
            errors.at(errors.size()-1).SuggestedReplacements.push_back(env->GetStringUTFChars(s,0));
        }
    }
}

LanguageTool* LanguageTool::getInstance(){
    if(instance==nullptr)
        instance = new LanguageTool;
    return instance;
}

void LanguageTool::getLanguages(vector<wxString> &languages){
	jobjectArray langs = (jobjectArray)env->CallObjectMethod(class_LanguageToolFacade, LanguageToolFacade_getLanguages);
	jsize len = env->GetArrayLength(langs);
	for (int i = 0; i < len; i++){
		jstring s = (jstring)env->GetObjectArrayElement(langs, i);
		languages.push_back(env->GetStringUTFChars(s, 0));
	}
}

bool LanguageTool::setLanguage(const wxString &language){
	jstring lang = env->NewStringUTF(language.c_str());
	jboolean result = (jboolean)env->CallStaticBooleanMethod(class_LanguageToolFacade, LanguageToolFacade_setLanguage, lang);
	return (bool)result;
}

LanguageTool::~LanguageTool(){
	if (jvm){
		jvm->DestroyJavaVM();
		delete jvm;
	}
	if (env)
		delete env;
}
