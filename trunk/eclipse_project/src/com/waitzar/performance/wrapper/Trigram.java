package com.waitzar.performance.wrapper;

public class Trigram {
	
	private Myanmar[] val = new Myanmar[3]; 
	
	public Trigram(Myanmar wordIsans2, Myanmar wordIsans1, Myanmar wordI) {
		val[0] = wordIsans2;
		val[1] = wordIsans1;
		val[2] = wordI;
	}
	
	public String toString() {
		return val[0] + "-" + val[1] + "-" + val[2];
	}
	
	public boolean equals(Object obj) {
		Trigram t = (Trigram)obj;
		return val[0].equals(t.val[0]) && val[1].equals(t.val[1]) && val[2].equals(t.val[2]);
	}
	
	public boolean equals(Trigram t, boolean[] useVals) {
		boolean isEqual = true;
		if (useVals[0] && !(val[0].equals(t.val[0])))
			isEqual = false;
		if (useVals[1] && !(val[1].equals(t.val[1])))
			isEqual = false;
		if (useVals[2] && !(val[2].equals(t.val[2])))
			isEqual = false;
		
		return isEqual;
	}


}
