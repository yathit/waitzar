package com.waitzar.analysis;


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
	//The actual string the user entered
	private String rawText;
	
	//The "canonically" equivalent string, unified to arbitrarily-chosen code points.
	private String canonText;
	
	public ZawgyiWord(String zawgyiText) {
		//Even invalid sequences still have representable text. 
		this.rawText = zawgyiText;
		
		//Ok, scan this string. 
	}
	
	/**
	 * Returns the raw string, as typed by the user.
	 */
	public String toString() {
		return rawText;
	}
	
	
	//Useful static functions
	public enum CHARACTER_WIDTH {
		NOT_MYANMAR, UNKNOWN, ZERO_WIDTH, SOME_WIDTH
	};
	
	public static CHARACTER_WIDTH getWidthClassifier(char mmChar) {
		if (mmChar<'\u1000' || mmChar>'\u109F')
			return CHARACTER_WIDTH.NOT_MYANMAR;
		
	}

}









