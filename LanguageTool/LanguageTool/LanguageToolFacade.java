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
package org.languagetool.facade;

import org.languagetool.JLanguageTool;
import org.languagetool.rules.RuleMatch;

import java.io.IOException;
import java.util.List;
import org.languagetool.Language;
import org.languagetool.Languages;


/**
 * A simple interactive test to see if the languages are available and don't crash.
 */
public class LanguageToolFacade {

  static JLanguageTool langTool;
  static Language language;
  public static boolean setLanguage(String name){
      language = Languages.getLanguageForName(name);
      if(Languages.isLanguageSupported(language.getShortCode())){
        langTool = new JLanguageTool(language);
        return true;
      } else {
          language = null;
          return false;
      }
  }
  
  public static String[] getLanguages(){
      List<Language> list = Languages.get();
      String[] languages = new String[list.size()];
      for(int i=0;i<list.size();i++){
          languages[i] = list.get(i).getName();
      }
      return languages;
  }

  public static RuleMatch[] checkText(String input){
    List<RuleMatch> result;
    if(language!=null){
        try {
            result = langTool.check(input);
            if(result!=null){
                RuleMatch[] array = new RuleMatch[result.size()];
                array = result.toArray(array);
                return array;
            }
            else return new RuleMatch[0];
        }
        catch (IOException e){
            return new RuleMatch[0];    
        }
    }
    return new RuleMatch[0];
 }
}
