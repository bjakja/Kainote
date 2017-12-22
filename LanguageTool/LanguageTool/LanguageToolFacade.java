/* LanguageTool, a natural language style checker 
 * Copyright (C) 2013 Daniel Naber (http://www.danielnaber.de)
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
package org.languagetool.facade;

import org.languagetool.JLanguageTool;
import org.languagetool.rules.RuleMatch;

import java.io.IOException;
import java.util.List;
import org.languagetool.Language;
import org.languagetool.Languages;
import org.languagetool.MultiThreadedJLanguageTool;


/**
 * A simple interactive test to see if the languages are available and don't crash.
 */
public class LanguageToolFacade {

  static MultiThreadedJLanguageTool langTool;
  public static boolean setLanguage(String name){
      langTool = new MultiThreadedJLanguageTool(Languages.getLanguageForName(name));
      return true;
  }
  
  public static String[] getLanguages(){
      List<Language> list = Languages.get();
      String[] languages = new String[list.size()];
      for(int i=0;i<list.size();i++){
          languages[i] = list.get(i).getName();
      }
      return languages;
  }

  public static RuleMatch[] checkText(String input) throws IOException {
    List<RuleMatch> result = langTool.check(input);
    if(result!=null){
        RuleMatch[] array = new RuleMatch[result.size()];
        array = result.toArray(array);
        return array;
    }
    else return new RuleMatch[0];
  }
  
}
