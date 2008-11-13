package com.waitzar.utility;

import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.regex.Matcher;

import com.waitzar.analysis.Action;
import com.waitzar.analysis.Combiner;
import com.waitzar.analysis.ZawgyiWord;


/**
 * This class exists to point out cases in which "ya-yit" and "ya-pin" do 
 *  not share the same romanisation, as they should. This would have been noticed
 *  earlier, except that the two sort into different locations.
 * @author Seth N. Hetu
 */
public class ThePeculiarDuplicityOfYaYit {

	private static Hashtable<ZawgyiWord, String> rhymes;
	

	public static void main(String[] args) {
		rhymes = new Hashtable<ZawgyiWord, String>();
		
		Combiner.readLines("rhymes.zg.txt", new Action() {
			public void perform(String line) {
				Matcher m = Combiner.equalsPattern.matcher(line);
				if (!m.matches()) {
					throw new RuntimeException("Line doesn't match: " + ZawgyiWord.printMM(line));
				}
				
				try {
					rhymes.put(new ZawgyiWord(m.group(1).trim()), m.group(2).trim());
				} catch (RuntimeException ex) {}
			};
		});
		
		//Open the output file
		BufferedWriter outFile = null;
		String outputFileName = "rhymes.inconsistent.txt";
		try {
			outFile = new BufferedWriter(new PrintWriter(outputFileName, "UTF-8"));
		} catch (FileNotFoundException ex) {
			System.out.println("Cannot output to file: " + outputFileName);
		} catch (UnsupportedEncodingException ex) {
			System.out.println("Out file encoding not supported (UTF-8).");
		}
		
		//Now...
		for (ZawgyiWord zg : rhymes.keySet()) {
			//Does this word contain ya-pin?
			if (zg.toString().contains("\u103A")) {
				//Find a similar one
				ZawgyiWord zgTest = new ZawgyiWord(zg.toString().replace('\u103A', '\u103B'));
				for (ZawgyiWord zg2 : rhymes.keySet()) {
					if (zg2.toCanonString().equals(zgTest.toCanonString())) {
						if (!rhymes.get(zg2).equals(rhymes.get(zg))) {
							try {
								outFile.write(zg.toString() + " = " + rhymes.get(zg));
								outFile.write("  ---  ");
								outFile.write(zg2.toString() + " = " + rhymes.get(zg2));
								outFile.write("\n");
							} catch (IOException ex) {
								System.out.println("Error!");
								ex.printStackTrace();
								System.exit(1);
							}
						}
					}
				}
			}
		}
		
		try {
			outFile.close();
		} catch (IOException ex) {}
		
		System.out.println("Done");
	}

}
