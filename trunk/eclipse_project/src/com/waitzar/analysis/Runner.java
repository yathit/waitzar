package com.waitzar.analysis;

import java.util.ArrayList;
import java.util.HashMap;

public class Runner {

	public static void main(String[] args) {
		HashMap<String, ArrayList<String>> rhymes = new HashMap<String, ArrayList<String>>();
		
		//NOTE: Take the two "contractions" out of the dictionary and make them their own special cases...
		
		
		
		//Read all rhymes, and do:
		//rhymes.get(word.toCanonString()).add(word);
		//we should save the "examples", too...
		
		//Then, output:
		//foreach canon : rhymes.keys
		//  if (canon.containsChar('('))
		//    continue;
		//  foreach word : rhymes.get(canon) { print it, & its examples }
		//  print a trailing newline
		//print "Errors:"
		//foreach (canon : rhymes.keys
		//  if (!canon.containsChar('('))
		//    continue;
		//  print: canon
		//  foreach word : rhymes.get(canon) { print it }
		
		

	}

}
