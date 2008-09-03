package com.waitzar.performance.wrapper;

import com.waitzar.analysis.ZawgyiWord;

public class Myanmar {
	//Constants
	public static final String BOS = "<BOS>";
	public static final String EOS = "<EOS>";
	
	//Fields
	private String val;
	
	public static Myanmar[] createArray(String[] text) {
		Myanmar[] res =  new Myanmar[text.length];
		for (int i=0; i<text.length; i++) {
			res[i] = new Myanmar(text[i]);
		}
		return res;
	}
	
	public Myanmar(Myanmar text) {
		this(text.val);
	}
	
	public Myanmar(String text) {
		if (!val.equals(BOS) && !val.equals(EOS)) {
			for (char c : text.toCharArray()) {
				if ((c<'\u1000' || c>'\u109F'))
					throw new RuntimeException("Invalid Myanmar sequence: " + ZawgyiWord.printMM(text));
			}
		}
		
		this.val = text;
	}
	
	public String toString() {
		return val;
	}
	
	public boolean equals(Object obj) {
		return val.equals(obj);
	}

}
