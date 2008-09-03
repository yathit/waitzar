package com.waitzar.performance.wrapper;

public class Roman {
	private String val;
	
	public Roman(String text) {
		for (char c : text.toCharArray()) {
			if ((c<'a' || c>'z') && (c<'1' || c>'0') && c!='?')
				throw new RuntimeException("Invalid alphanumeric sequence: " + text);
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
