package com.waitzar.analysis;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class Runner {

	public static void main(String[] args) {
		HashMap<String, ArrayList<ZawgyiWord>> rhymes = new HashMap<String, ArrayList<ZawgyiWord>>();

		String fileName = "rhymes.txt";
		if (args.length > 0) {
			fileName = args[0];
		}

		//Open the file
		BufferedReader reader = null;
		try {
			reader = new BufferedReader(new InputStreamReader(new FileInputStream(fileName), "UTF-8"));
		} catch (FileNotFoundException ex) {
			System.out.println("File not found.");
			return;
		} catch (UnsupportedEncodingException ex1) {
			System.out.println("Unsupported encoding (UTF-8).");
			return;
		}


		//Read each line
		Pattern linePattern = Pattern.compile("([^ ]+)([^\t]+\t[^\t]+\t[^\t]+)");
		try {
			for (String line = reader.readLine(); line!=null; line=reader.readLine()) {
				if (line.length()==0)
					continue;
				line = line.trim();

				//Parse
				Matcher m = linePattern.matcher(line);
				if (!m.matches()) {
					throw new RuntimeException("Line doesn't match: " + ZawgyiWord.printMM(line));
				}
				//System.out.println("Raw: " + ZawgyiWord.printMM(m.group(1)));
				ZawgyiWord rhyme = new ZawgyiWord(m.group(1));
				rhyme.setData(m.group(2));

				//Add it
				if (!rhymes.containsKey(rhyme.toCanonString())) {
					rhymes.put(rhyme.toCanonString(), new ArrayList<ZawgyiWord>());
				}
				rhymes.get(rhyme.toCanonString()).add(rhyme);
			}
			reader.close();
		} catch (IOException ex) {
			System.out.println("IO exception");
		}

		//Now, sort our key set. We have to meddle a bit with this one...
		Set<String> keys = rhymes.keySet();
		ArrayList<ZawgyiWord> firstWords = new ArrayList<ZawgyiWord>();
		for (String key : keys) {
			ZawgyiWord firstWord = rhymes.get(key).get(0);
			firstWords.add(firstWord);
		}
		Collections.sort(firstWords, ZawgyiWord.getComparator());

		//Open output file
		BufferedWriter writeFile = null;
		int totalRhymes = 0;
		int uniqueRhymes = 0;

		try {
			writeFile = new BufferedWriter(new PrintWriter(fileName + ".ordered", "UTF-8"));
		} catch (FileNotFoundException ex) {
			System.out.println("Out file not found.");
		} catch (UnsupportedEncodingException ex) {
			System.out.println("Out file encoding not supported (UTF-8).");
		}

		//Print... saving unknowns
		try {
			ArrayList<ZawgyiWord> unknown = new ArrayList<ZawgyiWord>();
			for (ZawgyiWord key : firstWords) {
				uniqueRhymes++;
			//	writeFile.write(key.toCanonString()+"\n");
				for (ZawgyiWord val : rhymes.get(key.toCanonString())) {
					totalRhymes++;
					if (val.getUnknown().length()==0) {
						writeFile.write(val.toString() + val.getData() + "\n");
					} else {
						unknown.add(val);
					}
				}
				writeFile.write("\n");
			}

			//Print unknowns
			if (unknown.size()>0)
				writeFile.write("\nUNKNOWNS\n");
			for (ZawgyiWord uk : unknown)
				writeFile.write(uk.toString() + " (" + uk.getUnknown() + ")\n");

			//Print stats
			writeFile.write("\nSTATS\n");
			writeFile.write("Unique Rhymes: " + uniqueRhymes + "\n");
			writeFile.write("Total Rhymes: " + totalRhymes + "\n");

			writeFile.close();
		} catch (IOException ex) {
			System.out.println("IO exception on output.");
		}

		System.out.println("Done");
	}

}
