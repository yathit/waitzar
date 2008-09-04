package com.waitzar.performance.wrapper;

public class Bigram {
	
	private Myanmar[] val = new Myanmar[2]; 
	
	public double doubleComponent;
	
	public Bigram(Myanmar prev, Myanmar curr) {
		val[0] = prev;
		val[1] = curr;
	}
	
	public String toString() {
		return val[0] + "-" + val[1];
	}
	
	public boolean equals(Object obj) {
		Bigram t = (Bigram)obj;
		return val[0].equals(t.val[0]) && val[1].equals(t.val[1]);
	}
	
	public Myanmar getUltimateWord() {
		return val[0];
	}
	
	public Myanmar getWord() {
		return val[1];
	}
	
	public boolean equals(Bigram t, boolean[] useVals) {
		boolean isEqual = true;
		if (useVals[0] && !(val[0].equals(t.val[0])))
			isEqual = false;
		if (useVals[1] && !(val[1].equals(t.val[1])))
			isEqual = false;
		
		return isEqual;
	}
	



}
