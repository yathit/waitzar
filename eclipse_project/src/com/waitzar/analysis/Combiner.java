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
import java.util.Collections;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class Combiner {
	
	public static HashMap<String, String> onsets = new HashMap<String, String>();
	public static HashMap<String, String> rhymes = new HashMap<String, String>();
	public static HashMap<String, String> specials = new HashMap<String, String>();
	
	public static ArrayList<ZawgyiWord> dictionary = new ArrayList<ZawgyiWord>();
	public static ArrayList<String> dictionaryUnsorted = new ArrayList<String>();
	
	public static Pattern equalsPattern = Pattern.compile("([^=]+)=(.+)");
	

	public static void main(String[] args) {
		//Read
		readFiles(args);
		
		//Process
		recombine();
	}
	
	
	public static void recombine() {
		BufferedWriter writeFile = null;
		try {
			writeFile = new BufferedWriter(new PrintWriter("wordlist.generated.txt", "UTF-8"));
		} catch (FileNotFoundException ex) {
			System.out.println("Out file not found.");
		} catch (UnsupportedEncodingException ex) {
			System.out.println("Out file encoding not supported (UTF-8).");
		}
		
		for (ZawgyiWord wordZG : dictionary) {
			dictionaryUnsorted.add(wordZG.toString());
		}
		
		int count = 0;
		for (String word : dictionaryUnsorted) {
			//Special cases:
			if (specials.containsKey(word)) {
				try {
					writeFile.write(word + " = " + specials.get(word) + "\n");
				} catch (IOException ex) {
					System.out.println("Error writing: " + ex.toString());
				}
				
				count++;
				continue;
			}
			
			
			//Get the onset
			String onset = "";
			for (String onsOpt : onsets.keySet()) {
				if (word.replaceAll(onsOpt+"\u1039", "").contains(onsOpt) && onsOpt.length()>onset.length()) {
					onset = onsOpt;
				}
			}
			
			//Get the rhyme
			String rhyme = word.replaceFirst(onset, "-");
			if (!rhymes.containsKey(rhyme)){
				throw new RuntimeException("["+count+"/"+dictionaryUnsorted.size()+"] Cannot find word for: " + ZawgyiWord.printMM(onset) + " + " + ZawgyiWord.printMM(rhyme));
			}
			
			//Print this option in our dictionary
			try {
				writeFile.write(word + " = " + onsets.get(onset) + rhymes.get(rhyme) + "\n");
			} catch (IOException ex) {
				System.out.println("Error writing: " + ex.toString());
			}
			
			count++;
		}
		
		try {
			writeFile.close();
		} catch (IOException ex) {}
		
		System.out.println("Done");
	}
	
	
	public static void readFiles(String[] args) {
		//Read all onsets...
		String onsetFile = "onsets.zg.txt";
		if (args.length > 0) {
			onsetFile = args[0];
		}
		readLines(onsetFile, new Action() {
			public void perform(String line) {
				Matcher m = equalsPattern.matcher(line);
				if (!m.matches()) {
					throw new RuntimeException("Line doesn't match: " + ZawgyiWord.printMM(line));
				}
				
				onsets.put(m.group(1).trim(), m.group(2).trim());
			};
		});
		
		
		String rhymeFile = "rhymes.zg.txt";
		if (args.length > 1) {
			rhymeFile = args[1];
		}
		readLines(rhymeFile, new Action() {
			public void perform(String line) {
				Matcher m = equalsPattern.matcher(line);
				if (!m.matches()) {
					throw new RuntimeException("Line doesn't match: " + ZawgyiWord.printMM(line));
				}
				
				rhymes.put(m.group(1).trim(), m.group(2).trim());
			};
		});
		

		String specialFile = "specials.zg.txt";
		if (args.length > 2) {
			specialFile = args[2];
		}
		readLines(specialFile, new Action() {
			public void perform(String line) {
				Matcher m = equalsPattern.matcher(line);
				if (!m.matches()) {
					throw new RuntimeException("Line doesn't match: " + ZawgyiWord.printMM(line));
				}
				
				specials.put(m.group(1).replaceAll("\\$", "").trim(), m.group(2).trim());
			};
		});
		

		String wordListFile = "words.zg.txt";
		if (args.length > 3) {
			wordListFile = args[3];
		}
		readLines(wordListFile, new Action() {
			public void perform(String line) {
				try {
					dictionary.add(new ZawgyiWord(line.trim()));
				} catch (RuntimeException ex) {
					dictionaryUnsorted.add(line.trim());
				}
			};
		});
		Collections.sort(dictionary, ZawgyiWord.getComparator());
	}
	

	
	public static void readLines(String filePath, Action lineAction) {
		BufferedReader reader = null;
		try {
			reader = new BufferedReader(new InputStreamReader(new FileInputStream(filePath), "UTF-8"));
		} catch (FileNotFoundException ex) {
			System.out.println("File not found.");
			return;
		} catch (UnsupportedEncodingException ex1) {
			System.out.println("Unsupported encoding (UTF-8).");
			return;
		}


		//Read each line
		try {
			for (String line = reader.readLine(); line!=null; line=reader.readLine()) {
				line = line.trim();
				if (line.length()==0 || line.startsWith("#"))
					continue;
				
				lineAction.perform(line);
			}
		} catch (IOException ex) {
			System.out.println("IO exception: " + ex.toString());
			return;
		}
		
		try {
			reader.close();
		} catch (IOException ex) {}
	}
	
	
}
