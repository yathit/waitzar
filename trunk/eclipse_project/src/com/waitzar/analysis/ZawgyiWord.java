package com.waitzar.analysis;

import java.util.regex.Matcher;
import java.util.regex.Pattern;


/**
 * This class exists to help resolve a series of issues with Zawgyi:
 *  1) Many zero-width characters in a row (1039+1039) might be illegal and difficult to detect visually.
 *  2) When a sequence of zero-width characters IS valid, a strict ordering is imposed to prevent
 *     having one sequence treated as two distinct ones.
 *  3) Zawgyi illegally disunifies several characters. Even getting past that, Burmese with no dialects
 *     disunifies short/long "ar". This class can detect such collisions, and deal with them
 *     when necessary (e.g., for sorting).
 * @author Seth N. Hetu
 */
public class ZawgyiWord {
	//Regexes
	public static Pattern consonantRegex = Pattern.compile("([\\u1000-\\u1021[\\u108F[\\u104C-\\u104F]]])[^\\u1039]");
	public static Pattern medialRegex = Pattern.compile("((\\-\\u103A)|(\\u103B\\-)|(\\u107E\\-)|(\\-\\u103C)|(\\-\\u103D)|(\\-\\u103C\\u103A)|(\\-\\u103D\\u103A)|(\\u1081\\-\\u103D)|(\\u1082\\-\\u103D)|(\\-\\u108A)|(\\u1081\\-\\u108A)|(\\u1082\\-\\u108A))");

	//The actual string the user entered
	private String rawText;

	//Sorting elements
	private String sortConsonant = "";
	private String sortMedial = "";
	private String sortFinal = "";
	private String sortVowel = "";
	private String sortTone = "";
	private String sortUnknown = "";

	public ZawgyiWord(String zawgyiText) {
		//Before anything, re-order the string where necessary.
		ZawgyiWord.properlyOrder(zawgyiText);

		//Even invalid sequences still have representable text.
		this.rawText = zawgyiText;

		//Errors?
		if (ZawgyiWord.isInError(zawgyiText))
			return;

		//Handle independent vowels and special contractions
		String[] independentVowels = new String[] {"\u1023", "\u1024", "\u1025", "\u1026", "\u1025\u102E", "\u1027", "\u1029", "\u107E\u101E", "\u102A", "\u1031\u107E\u101E\u102C\u1039"};
		String[] collationForms = new String[] {"\u1021\u102D", "\u1021\u102E", "\u1021\u102F", "\u1021\u1030", "\u1021\u1030", "\u1021\u1031", "\u1031\u1021\u102C", "\u1031\u1021\u102C", "\u1031\u1021\u102C\u1039", "\u1031\u1021\u102C\u1039"};

		for (int i=0; i<independentVowels.length(); i++) {
			if (this.rawText.contains(independentVowels[i])) {
				segmentWord(this.rawText.replaceAll(independentVowels[i], collationForms[i]);
				break;
			} else if (i==independentVowels.length()-1) {
				if (this.rawText.equals("\u1031\u101A\u102C\u1000\u1039\u103A\u102C\u1038"))
					segmentWord("\u101A\u102C\u1000\u1039\u1000\u103A\u102C\u1038");
				else if (this.rawText.equals("\u1000\u103C\u103A\u108F\u102F\u1039\u1015\u1039"))
					segmentWord("\u1000\u103C\u103A\u108F\u102F\u1014\u102F\u1015\u1039");
				else
					segmentWord(this.rawText);
			}
		}
	}


	/**
	 * Populates consonant/medial/vowell etc. fields
	 * @param text
	 */
	private void segmentWord(String text) {
		//Special case for WZ: allow "-" as a consonant
		if (text.contains("-"))
			sortConsonant = "-";
		else {
			//Figure out the consonant, replace with a "-"
			Matcher m = consonantRegex.matcher(text);
			if (m.matches()) {
				sortConsonant = m.group(1);
				text = text.replaceFirst(sortConsonant, "-");
				sortConsonant = sortConsonant.replaceAll("\u108F", "\u1014");
			}
		}

		//Segment medial
		if (text.length()>1) {
			Matcher m = medialRegex.matcher(text);
			if (m.matches()) {
				sortMedial = m.group(1);
				text = text.replaceFirst(sortMedial, "-");
				sortMedial = sortMedial.replaceAll("\u1081|\u1082|\u107E", "\u103B");
				sortMedial = sortMedial.replaceAll("\u108A", "\u103D\u103C");
			} else {
				sortMedial = "-";
			}
		}

		//Segment vowell

		//Segment final

		//Segment tone







		//Finally
		if (text.length()>1) {
			sortUnknown = text;
		}

	}


	/**
	 * Returns the raw string, as typed by the user.
	 */
	public String toString() {
		return rawText;
	}

	/**
	 * Returns the string used for sorting.
	 */
	public String toCanonString() {
		if (sortUnknown.length()==0)
			return sortConsonant + sortMedial + sortFinal + sortVowel + sortTone;
		else
			return sortConsonant + sortMedial + sortFinal + sortVowel + sortTone + "(" + sortUnknown + ")";
	}


	//Useful static functions
	public static void properlyOrder(String txt) {
		//Re-order where necessary.
		char[] chars = txt.trim().toCharArray();

		//Sort
		for (int chIn=0; chIn<chars.length; chIn++) {
			//Properly order: 102F 102D
			if (chIn>0 && chars[chIn-1]==0x102D && chars[chIn]==0x102F) {
				chars[chIn-1]=0x102F;
				chars[chIn]=0x102D;
			}
			//Properly order: 103A 102D
			if (chIn>0 && chars[chIn-1]==0x102D && chars[chIn]==0x103A) {
				chars[chIn-1]=0x103A;
				chars[chIn]=0x102D;
			}
			//Properly order: 103D 102D
			if (chIn>0 && chars[chIn-1]==0x102D && chars[chIn]==0x103D) {
				chars[chIn-1]=0x103D;
				chars[chIn]=0x102D;
			}
			//Properly order: 1075 102D
			if (chIn>0 && chars[chIn-1]==0x102D && chars[chIn]==0x1075) {
				chars[chIn-1]=0x1075;
				chars[chIn]=0x102D;
			}
			//Properly order: 102D 1087
			if (chIn>0 && chars[chIn-1]==0x1087 && chars[chIn]==0x102D) {
				chars[chIn-1]=0x102D;
				chars[chIn]=0x1087;
			}
			//Properly order: 103D 102E
			if (chIn>0 && chars[chIn-1]==0x102E && chars[chIn]==0x103D) {
				chars[chIn-1]=0x103D;
				chars[chIn]=0x102E;
			}
			//Properly order: 103D 103A
			if (chIn>0 && chars[chIn-1]==0x103A && chars[chIn]==0x103D) {
				chars[chIn-1]=0x103D;
				chars[chIn]=0x103A;
			}
			//Properly order: 1039 103A -Note that this won't actually merge this fix!
			// Possibly set merged = true... ?
			if (chIn>0 && chars[chIn-1]==0x103A && chars[chIn]==0x1039) {
				chars[chIn-1]=0x1039;
				chars[chIn]=0x103A;
			}
			//Properly order: 1030 102D
			if (chIn>0 && chars[chIn-1]==0x102D && chars[chIn]==0x1030) {
				chars[chIn-1]=0x1030;
				chars[chIn]=0x102D;
			}
			//Properly order: 1037 1039
			if (chIn>0 && chars[chIn-1]==0x1039 && chars[chIn]==0x1037) {
				chars[chIn-1]=0x1037;
				chars[chIn]=0x1039;
			}
			//Properly order: 1032 1037
			if (chIn>0 && chars[chIn-1]==0x1037 && chars[chIn]==0x1032) {
				chars[chIn-1]=0x1032;
				chars[chIn]=0x1037;
			}
			//Properly order: 1032 1094
			if (chIn>0 && chars[chIn-1]==0x1094 && chars[chIn]==0x1032) {
				chars[chIn-1]=0x1032;
				chars[chIn]=0x1094;
			}
			//Properly order: 1064 1094
			if (chIn>0 && chars[chIn-1]==0x1094 && chars[chIn]==0x1064) {
				chars[chIn-1]=0x1064;
				chars[chIn]=0x1094;
			}
			//Properly order: 102D 1094
			if (chIn>0 && chars[chIn-1]==0x1094 && chars[chIn]==0x102D) {
				chars[chIn-1]=0x102D;
				chars[chIn]=0x1094;
			}
			//Properly order: 102D 1071
			if (chIn>0 && chars[chIn-1]==0x1071 && chars[chIn]==0x102D) {
				chars[chIn-1]=0x102D;
				chars[chIn]=0x1071;
			}
			//Properly order: 1036 1037
			if (chIn>0 && chars[chIn-1]==0x1037 && chars[chIn]==0x1036) {
				chars[chIn-1]=0x1036;
				chars[chIn]=0x1037;
			}
			//Properly order: 1036 1088
			if (chIn>0 && chars[chIn-1]==0x1088 && chars[chIn]==0x1036) {
				chars[chIn-1]=0x1036;
				chars[chIn]=0x1088;
			}
			//Properly order: 1039 1037
			// ###NOTE: I don't know how [XXXX][1037][1039] can parse correctly...
			if (chIn>0 && chars[chIn-1]==0x1037 && chars[chIn]==0x1039) {
				chars[chIn-1]=0x1039;
				chars[chIn]=0x1037;
			}
			//Properly order: 102D 1033
			//NOTE that this is later reversed for "103A 1033 102D" below
			// Also for 103C 1033 102D, what a mess...
			if (chIn>0 && chars[chIn-1]==0x1033 && chars[chIn]==0x102D) {
				chars[chIn-1]=0x102D;
				chars[chIn]=0x1033;
			}
			//Properly order: 103C 1032
			if (chIn>0 && chars[chIn-1]==0x1032 && chars[chIn]==0x103C) {
				chars[chIn-1]=0x103C;
				chars[chIn]=0x1032;
			}
			//Properly order: 103C 102D
			if (chIn>0 && chars[chIn-1]==0x102D && chars[chIn]==0x103C) {
				chars[chIn-1]=0x103C;
				chars[chIn]=0x102D;
			}
			//Properly order: 103C 102E
			if (chIn>0 && chars[chIn-1]==0x102E && chars[chIn]==0x103C) {
				chars[chIn-1]=0x103C;
				chars[chIn]=0x102E;
			}
			//Properly order: 1036 102F
			if (chIn>0 && chars[chIn-1]==0x102F && chars[chIn]==0x1036) {
				chars[chIn-1]=0x1036;
				chars[chIn]=0x102F;
			}
			//Properly order: 1036 1088
			if (chIn>0 && chars[chIn-1]==0x1088 && chars[chIn]==0x1036) {
				chars[chIn-1]=0x1036;
				chars[chIn]=0x1088;
			}
			//Properly order: 1036 103D
			if (chIn>0 && chars[chIn-1]==0x103D && chars[chIn]==0x1036) {
				chars[chIn-1]=0x1036;
				chars[chIn]=0x103D;
			}
			//Properly order: 1036 103C
			if (chIn>0 && chars[chIn-1]==0x103C && chars[chIn]==0x1036) {
				chars[chIn-1]=0x1036;
				chars[chIn]=0x103C;
			}
			//Properly order: 107D 103C
			if (chIn>0 && chars[chIn-1]==0x103C && chars[chIn]==0x107D) {
				chars[chIn-1]=0x107D;
				chars[chIn]=0x103C;
			}
			//Properly order: 1088 102D
			if (chIn>0 && chars[chIn-1]==0x102D && chars[chIn]==0x1088) {
				chars[chIn-1]=0x1088;
				chars[chIn]=0x102D;
			}
			//Properly order: 1019 107B 102C
			//ASSUME: 1019 is stationary
			if (chIn>1 && chars[chIn-2]==0x1019 && chars[chIn-1]==0x102C && chars[chIn]==0x107B) {
				chars[chIn-1]=0x107B;
				chars[chIn]=0x102C;
			}
			//Properly order: 103A 1033 102D
			//NOTE that this directly overrides "102D 1033" as entered above
			//ASSUME: 103A is stationary
			if (chIn>1 && chars[chIn-2]==0x103A && chars[chIn-1]==0x102D && chars[chIn]==0x1033) {
				chars[chIn-1]=0x1033;
				chars[chIn]=0x102D;
			}
			//Properly order: 103C 102D 1033
			if (chIn>1 && chars[chIn-2]==0x103C && chars[chIn-1]==0x1033 && chars[chIn]==0x102D) {
				chars[chIn-1]=0x102D;
				chars[chIn]=0x1033;
			}
			//Properly order: 1019 107B 102C 1037
			if (chIn>2 && chars[chIn-3]==0x1019 &&
					(   (chars[chIn-2]==0x107B && chars[chIn-1]==0x1037 && chars[chIn]==0x102C)
					  ||(chars[chIn-2]==0x102C && chars[chIn-1]==0x107B && chars[chIn]==0x1037)
					  ||(chars[chIn-2]==0x102C && chars[chIn-1]==0x1037 && chars[chIn]==0x107B)
					  ||(chars[chIn-2]==0x1037 && chars[chIn-1]==0x107B && chars[chIn]==0x102C)
					)) {
				chars[chIn-2]=0x107B;
				chars[chIn-1]=0x102C;
				chars[chIn]=0x1037;
			}

			//Properly order: 107E XXXX 1036 1033
			if (chIn>2 && chars[chIn-3]==0x107E && chars[chIn-1]==0x1033 && chars[chIn]==0x1036) {
				chars[chIn-1]=0x1036;
				chars[chIn]=0x1033;
			}

			//FIX: [103B-->1081 XXXX 103C] and [107E-->1082 XXXX 103C]
			if (chIn>1 && chars[chIn]==0x103C) {
				if (chars[chIn-2]==0x103B)
					chars[chIn-2]=0x1081;
				else if (chars[chIn-2]==0x107E)
					chars[chIn-2]=0x1082;
			}
			//FIX: [100A-->106B  108A]
			if (chIn>0 && chars[chIn]==0x108A && chars[chIn-1]==0x100A) {
				chars[chIn-1]=0x106B;
			}

			//Small fix 1072's a bit ugly at times
			if (chIn>0 && chars[chIn-1]==0x1010 && chars[chIn]==0x1072) {
				chars[chIn]=0x1071;
			}
		}

		txt = new String(chars);
	}


	public static boolean isInError(String txt) {
		//Check for bad sequences, etc.
		return false;
	}

	/*public enum CHARACTER_WIDTH {
		NOT_MYANMAR, UNKNOWN, ZERO_WIDTH, SOME_WIDTH
	};

	public static CHARACTER_WIDTH getWidthClassifier(char mmChar) {
		if (mmChar<'\u1000' || mmChar>'\u109F')
			return CHARACTER_WIDTH.NOT_MYANMAR;

	}*/

}









