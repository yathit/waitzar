package com.waitzar.analysis;

import java.util.Comparator;
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
	//public static Pattern consonantRegex = Pattern.compile("([\\u1000-\\u1021[\\u108F[\\u104C-\\u104F]]])[^\\u1039]");
	//public static Pattern medialRegex = Pattern.compile("((\\-\\u103A)|(\\u103B\\-)|(\\u107E\\-)|(\\-\\u103C)|(\\-\\u103D)|(\\-\\u103C\\u103A)|(\\-\\u103D\\u103A)|(\\u1081\\-\\u103D)|(\\u1082\\-\\u103D)|(\\-\\u108A)|(\\u1081\\-\\u108A)|(\\u1082\\-\\u108A))");

	//The actual string the user entered
	private String rawText;

	//Sorting elements
	private int sortConsonant = 0;
	private int sortMedial = 0;
	private int sortFinal = 0;
	private int sortVowel = 0;
	private int sortTone = 0;
	private String unknownBit = "";
	
	private static int count = 0;
	
	private String data = "";

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

		for (int i=0; i<independentVowels.length; i++) {
			if (this.rawText.contains(independentVowels[i]) && (!independentVowels[i].equals("\u1025") || !this.rawText.contains("\u1025\u1039"))) {
				segmentWord(this.rawText.replaceAll(independentVowels[i], collationForms[i]));
				break;
			} else if (i==independentVowels.length-1) {
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
	 * Populates consonant/medial/vowel etc. fields
	 * @param text
	 */
	private void segmentWord(String text) {
		System.out.println(count++ + " " + printMM(text));
		
		//First, merge all characters to one representation
		text = unifyText(text);

		//Special case for WZ: allow "-" as a consonant
		text = text.replaceAll("\\-", "");
		
		//Figure out the consonant, replace with a "-"
		text = extractConsonant(text);
		
		//Segment the medial next
		text = extractMedial(text);

		//Segment vowell
		text = extractVowel(text);

		//Segment final
		text = extractFinal(text);

		//Segment tone
		text = extractTone(text);

		//Finally
		if (text.length()>0) {
			unknownBit = text;
		}

	}

	
	private String extractTone(String text) {
		StringBuilder sb =  new StringBuilder();
		boolean foundDotBelow = false;
		boolean foundVisarga = false;
		
		for (int i=0; i<text.length(); i++) {
			char c = text.charAt(i);
			if (c=='\u1037') {
				if (foundDotBelow)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundDotBelow = true;
			} else if (c=='\u1038') {
				if (foundVisarga)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundVisarga = true;
			} else
				sb.append(c);
		}
		
		if (foundDotBelow && foundVisarga)
			sortTone = 3;
		else if (foundDotBelow)
			sortTone = 1;
		else if (foundVisarga)
			sortTone = 2;
		
		return sb.toString();
	}
	
	
	private String extractFinal(String text) {
		StringBuilder sb =  new StringBuilder();
		boolean foundAa = false;
		boolean foundI = false;
		boolean foundU = false;
		boolean foundE = false;
		boolean foundKilledAa = true;
		
		for (int i=0; i<text.length(); i++) {
			char c = text.charAt(i);
			
			//Special case for kinzi
			if (c=='\u1064') {
				if (sortFinal != 0) {
					throw new RuntimeException("Mixed final found on " + printMM(text));
				} else 
					sortFinal = 6;
			}
			
			//One exception, then proceed as normal, looking for killed characters
			int finalVal = 0;
			if (c=='\u1025' && i<text.length()-1 && text.charAt(i+1)=='\u1039')
				c = '\u100A';
			if (c>'\u1000' && c<'\u1020' && c!='\u101B' && c!='\u101D' && c!='\u101F') {
				if (i<text.length()-1 && text.charAt(i+1)=='\u1039') {
					if (c=='\u101C')
						finalVal = 28;
					else if (c=='\u101E')
						finalVal = 29;
					else if (c=='\u1020')
						finalVal = 30;
					else
						finalVal = c - '\u1000' + 1;
					i++; //Skip asat
				}
			} else 
				sb.append(c);
			
			//Set it
			if (finalVal!=0 && sortFinal!=0) 
				throw new RuntimeException("Final already set on : " + printMM(text));
			sortFinal = finalVal;
		}
		
		return sb.toString();
	}
	
	
	
	private String extractVowel(String text) {
		StringBuilder sb =  new StringBuilder();
		boolean foundAa = false;
		boolean foundI = false;
		boolean foundU = false;
		boolean foundUu = false;
		boolean foundE = false;
		boolean foundKilledAa = false;
		boolean foundAnusvara = false;
		
		for (int i=0; i<text.length(); i++) {
			char c = text.charAt(i);
			if (c=='\u102C') {
				if (i<text.length()-1 && text.charAt(i+1)=='\u1039') {
					if (foundKilledAa)
						throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
					else
						foundKilledAa = true;
					i++; //Skip asat
				} else {
					if (foundAa)
						throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
					else
						foundAa = true;
				}
			} else if (c=='\u102D') {
				if (foundI)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundI = true;
			} else if (c=='\u102F') {
				if (foundU)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundU = true;
			} else if (c=='\u1030') {
				if (foundUu)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundUu = true;
			} else if (c=='\u1031') {
				if (foundE)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundE = true;
			} else if (c=='\u1036') {
				if (foundAnusvara)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundAnusvara = true;
			} else if (c=='\u102E') {
				if (sortVowel!=0)
					throw new RuntimeException("Mixed vowel found on " + printMM(text));
				else
					sortVowel = 3;
			} else if (c=='\u1032') {
				if (sortVowel!=0)
					throw new RuntimeException("Mixed vowel found on " + printMM(text));
				else
					sortVowel = 7;
			} else {
				sb.append(c);
			}
		}
		
		//Finally...
		if ((foundAa || foundI || foundU || foundE) && sortVowel!=0)
			throw new RuntimeException("Double vowel found on " + printMM(text));
		
		if (foundI) {
			if (foundU) {
				sortVowel = 11;
			} else {
				sortVowel = 2;
			}
		} else if (foundU) {
			if (foundAnusvara) {
				sortVowel = 12;
			} else {
				sortVowel = 4;
			}
		} else if (foundE) {
			if (foundKilledAa) {
				sortVowel = 9;
			} else if (foundAa) {
				sortVowel = 8;
			} else {
				sortVowel = 6;
			}
		} else if (foundAa) {
			sortVowel = 1;
		} else if (foundUu) {
			if (foundAnusvara) {
				sortVowel = 13;
			} else {
				sortVowel = 5;
			}
		}
		
		return sb.toString();
	}
	
	
	private String extractMedial(String text) {
		StringBuilder sb =  new StringBuilder();
		boolean foundYa = false;
		boolean foundYeye = false;
		boolean foundWa = false;
		boolean foundHa = false;
		
		for (int i=0; i<text.length(); i++) {
			char c = text.charAt(i);
			if (c=='\u103A') {
				if (foundYa)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundYa = true;
			} else if (c=='\u103B') {
				if (foundYeye)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundYeye = true;
			} else if (c=='\u103C') {
				if (foundWa)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundWa = true;
			} else if (c=='\u103D') {
				if (foundHa)
					throw new RuntimeException("Already found (" + (int)c + ") in " + printMM(text));
				else
					foundHa = true;
			} else 
				sb.append(c);
		}
		
		//Set properly
		if (foundHa && foundWa) {
			if (foundYeye)
				sortMedial = 9;
			else
				sortMedial = 8;
		} else if (foundWa) {
			if (foundYeye)
				sortMedial = 7;
			else if (foundYa)
				sortMedial = 5;
			else
				sortMedial = 3;
		} else if (foundHa) {
			if (foundYa)
				sortMedial = 6;
			else
				sortMedial = 4;
		} else if (foundYa) {
			sortMedial = 1;
		} else if (foundYeye) {
			sortMedial = 2;
		}
		
		return sb.toString();
	}
	
	
	private String extractConsonant(String text) {
		//Consonants are somewhat tricky (since they can also appear in finals).
		StringBuilder sb =  new StringBuilder();
		for (int i=0; i<text.length(); i++) {
			char c = text.charAt(i);
			
			//Basic consonants first. Test for "asat" in all cases, even though it's probably only necessary in the first.
			int newConsonant = 0;			
			if (c>='\u1000' && c<='\u1021' && (i==text.length()-1 || text.charAt(i+1)!='\u1039')) {
				newConsonant = ((int)c) - 0x1000 + 1; 
			} else if (c=='\u103F' && (i==text.length()-1 || text.charAt(i+1)!='\u1039')) {
				newConsonant = 35;
			} else if (c>='\u104C' && c<='\u104F' && (i==text.length()-1 || text.charAt(i+1)!='\u1039')) {
				newConsonant = ((int)c) - 0x104C + 36;
			} else {
				sb.append(c);
			}
			
			//Set it?
			if (newConsonant!=0) {
				if (sortConsonant!=0) {
					throw new RuntimeException("Double consonant ("+(int)c+") in " + printMM(text));
				} else {
					sortConsonant = newConsonant;
				}
			}
		}
		
		return sb.toString();
	}
	
	public String unifyText(String text) {
		//First, some quick substitutions
		text = text.replaceAll("\u105A", "\u102C\u1039").replaceAll("\u1088", "\u103D\u102F").replaceAll("\u1089", "\u103D\u1030").replaceAll("\u108A", "\u103D\u103C");		
		
		char[] src = text.toCharArray();
		char[] res = new char[src.length];
		for (int i=0; i<src.length; i++) {
			//Handle error cases:
			char c = src[i];
			if   ( (c>='\u1023' && c<='\u1027' && c!='\u1025')
				|| (c>='\u1029' && c<='\u102A')
				|| (c>='\u1040' && c<='\u1049')
				|| (c>='\u104A' && c<='\u104B')
				|| (c>='\u1050' && c<='\u1059')
				|| (c>='\u1060' && c<='\u1063')
				|| (c>='\u1065' && c<='\u1068')
				|| (c=='\u1069')
				|| (c=='\u106C')
				|| (c=='\u106D')
				|| (c>='\u106E' && c<='\u106F')
				|| (c>='\u1070' && c<='\u107C')
				|| (c=='\u1085')
				|| (c>='\u108B' && c<='\u108E')
				|| (c>='\u1091' && c<='\u1092')
				|| (c>='\u1096' && c<='\u1097')
				|| (c=='\u109F')) {
				
				//Error
				throw new RuntimeException("Bad character range in: " + printMM(text));
			} else {
				switch (c) {
					case '\u1000':
					case '\u1001':
					case '\u1002':
					case '\u1003':
					case '\u1004':
					case '\u1005':
					case '\u1006':
					case '\u1007':
					case '\u1008':
					case '\u100B':
					case '\u100C':
					case '\u100D':
					case '\u100E':
					case '\u100F':
					case '\u1010':
					case '\u1011':
					case '\u1012':
					case '\u1013':
					case '\u1015':
					case '\u1016':
					case '\u1017':
					case '\u1018':
					case '\u1019':
					case '\u101A':
					case '\u101C':
					case '\u101D':
					case '\u101E':
					case '\u101F':
					case '\u1020':
					case '\u1021':
					case '\u102D':
					case '\u102E':
					case '\u1031':
					case '\u1032':
					case '\u1036':
					case '\u1038':
					case '\u1039':
					case '\u103C':
					case '\u104C':
					case '\u104D':
					case '\u104E':
					case '\u104F':
					case '\u1064':
					case '\u1086':
					case '-': //eek
					case '\u1025': //double-eek
						res[i] = c;
						break;
					case '\u1009':
					case '\u106A':
						res[i] = '\u1009';
						break;
					case '\u100A':
					case '\u106B':
						res[i] = '\u100A';
						break;
					case '\u1014':
					case '\u108F':
						res[i] = '\u1014';
						break;
					case '\u101B':
					case '\u1090':
						res[i] = '\u101B';
						break;
					case '\u102C':
					case '\u102B':
						res[i] = '\u102C';
						break;
					case '\u102F':
					case '\u1033':
						res[i] = '\u102F';
						break;
					case '\u1030':
					case '\u1034':
						res[i] = '\u1030';
						break;
					case '\u1037':
					case '\u1094':
					case '\u1095':
						res[i] = '\u1037';
						break;
					case '\u103A':
					case '\u107D':
						res[i] = '\u103A';
						break;
					case '\u103B':
					case '\u107E':
					case '\u107F':
					case '\u1080':
					case '\u1081':
					case '\u1082':
					case '\u1083':
					case '\u1084':
						res[i] = '\u103B';
						break;
					case '\u103D':
					case '\u1087':
						res[i] = '\u103D';
						break;

					default:
						//Unknown
						throw new RuntimeException("Unknown character (" + (int)c + ") in range: " + printMM(text));
				}
			}
		}
		
		return new String(res);
	}
	
	
	public static String printMM(String txt) {
		StringBuilder sb = new StringBuilder();
		for (char c : txt.toCharArray()) {
			if (c<'\u1000' || c>'\u109F')
				sb.append(c);
			else			
				sb.append("[").append(Integer.toHexString(c)).append("]");
		}
		return sb.toString();
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
		return "C" + sortConsonant + " M" +  sortMedial + " F" + sortFinal + " V" + sortVowel + " T" + sortTone;
	}
	
	public String getUnknown() {
		return unknownBit;
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
	
	public static int compare (ZawgyiWord word1, ZawgyiWord word2) {
		//first order: consonants
		if (word1.sortConsonant < word2.sortConsonant)
			return -1;
		else if (word1.sortConsonant > word2.sortConsonant)
			return 1;

		//second order: medials
		if (word1.sortMedial < word2.sortMedial)
			return -1;
		else if (word1.sortMedial > word2.sortMedial)
			return 1;
		
		//third order: finals
		if (word1.sortFinal < word2.sortFinal)
			return -1;
		else if (word1.sortFinal > word2.sortFinal)
			return 1;
		
		//fourth order: vowels
		if (word1.sortVowel < word2.sortVowel)
			return -1;
		else if (word1.sortVowel > word2.sortVowel)
			return 1;
		
		//fifth order: tones
		if (word1.sortTone < word2.sortTone)
			return -1;
		else if (word1.sortTone > word2.sortTone)
			return 1;
		
		//That's it.... unknown stuff should all sort into the "0" basket
		return 0;
	}
	
	public static Comparator<ZawgyiWord> getComparator() {
		return new Comparator<ZawgyiWord>() {
			public int compare(ZawgyiWord o1, ZawgyiWord o2) {
				return ZawgyiWord.compare(o1, o2);
			}
		};
	}
	
	public String getData() {
		return data;
	}
	public void setData(String val) {
		this.data = val;
	}

	/*public enum CHARACTER_WIDTH {
		NOT_MYANMAR, UNKNOWN, ZERO_WIDTH, SOME_WIDTH
	};

	public static CHARACTER_WIDTH getWidthClassifier(char mmChar) {
		if (mmChar<'\u1000' || mmChar>'\u109F')
			return CHARACTER_WIDTH.NOT_MYANMAR;

	}*/

}









