package com.waitzar.wordsearch;

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
import java.util.Hashtable;
import java.util.regex.Matcher;

import com.waitzar.analysis.Action;
import com.waitzar.analysis.Combiner;
import com.waitzar.analysis.ZawgyiWord;

public class WordSearcher {
	private HashMap<String, String> onsets = new HashMap<String, String>();
	private HashMap<String, String> rhymes = new HashMap<String, String>();
	private HashMap<String, String> specials = new HashMap<String, String>();
	private ArrayList<ZawgyiWord> dictionary = new ArrayList<ZawgyiWord>();
	private ArrayList<String> dictionaryUnsorted = new ArrayList<String>();
		
	private HashMap<String, int[]> dictionaryCounts = new HashMap<String, int[]>();
	private ArrayList<String> allWordsSortedLargestFirst = new ArrayList<String>();
	
	public WordSearcher(String[] args) {
		//Read model & corpus, compute counts
		readFiles(args);
		
		//Now, count up total words and report which rhymes must be listed to 
		// capture 90% of these.
		doCalculation();
	}
	
	
	private void doCalculation() {
		//First, process our strings
		Hashtable<String, String> wordToRhyme = new Hashtable<String, String>();
		Hashtable<String, ArrayList<String>> rhymeToWords = new Hashtable<String, ArrayList<String>>();
		ArrayList<String> wordsToCheck = new ArrayList<String>();
		
		System.out.println("Dictionary: " + dictionary.size());
		System.out.println("Un-Sorted Dictionary: " + dictionaryUnsorted.size());
		System.out.println("\"All Words\" Array: " + allWordsSortedLargestFirst.size());
		
		for (String currWord : allWordsSortedLargestFirst) {
			//Get the next relevant word
			if (currWord.length()>=1 && currWord.charAt(0)>='\u1040' && currWord.charAt(0)<='\u1049')
				continue;
			if (currWord.equals("\u1031\u101c\u102c\u1037\u1002\u1039"))
				continue;
			if (currWord.equals("\u1025") || currWord.equals("\u1027"))
				continue;
			if (currWord.equals("\u1015\u103c\u102d\u1033\u1004\u1039\u1037"))
				continue;
			if (currWord.equals("\u1027\u100a\u1039\u1037"))
				continue;
			if (specials.containsKey(currWord))
				continue;
			String currRhyme = null;
			try {
				ZawgyiWord zg = new ZawgyiWord(currWord);
			} catch (RuntimeException ex) {
				continue;
			}
			ArrayList<Character> noConsStr =  new ArrayList<Character>();
			for (int i=0; i<currWord.length(); i++) {
				char c = currWord.charAt(i);

				//Basic consonants first. Test for "asat" in all cases, even though it's probably only necessary in the first.
				if (c>='\u1000' && c<='\u1021' && (i==currWord.length()-1 || currWord.charAt(i+1)!='\u1039')) {
					if (i<currWord.length()-2) {
						//Handle three-letter words
						if ((c=='\u101E' || c=='\u101C') && currWord.charAt(i+1)=='\u103D' && currWord.charAt(i+2)=='\u103A')
							i+=2;
					} 
					if (i<currWord.length()-1) {
						//Handle two-letter consonants
						if (c=='\u1005' && currWord.charAt(i+1)=='\u103A')
							i++;
						else if (c=='\u101B' && currWord.charAt(i+1)=='\u103D')
							i++;
						else if (c=='\u101B' && currWord.charAt(i+1)=='\u108A')
							i++;
					}
					continue;
				} else if (c=='\u103F' && (i==currWord.length()-1 || currWord.charAt(i+1)!='\u1039')) {
					continue;
				} else if (c>='\u104C' && c<='\u104F' && (i==currWord.length()-1 || currWord.charAt(i+1)!='\u1039')) {
					continue;
				} else if ((c=='\u1024' || c=='\u1026' || c=='\u108f' || c=='\u1086' || c=='\u1090' || c=='\u1092' || c=='\u1097' || c=='\u1091' || c=='\u106A' || c=='\u106B') && (i==currWord.length()-1 || currWord.charAt(i+1)!='\u1039')) {
					//Special
					if (c=='\u1026')
						noConsStr.add('\u102E');
					continue;
				} else {
					noConsStr.add(c);
				}
			}
			
			//Now, check the rhymes list	
			OUTER:
			for (String rhyme : rhymes.keySet()) {				
				if (rhyme.length()-1 != noConsStr.size())
					continue;
				
				for (char c : rhyme.toCharArray()) {
					if (c == '-')
						continue;
					if (!noConsStr.contains(c)) {
						continue OUTER;
					}
				}
				
				currRhyme = rhyme;
				break;
			}
			
			if (currRhyme==null) {
				System.out.println("ERROR: No rhyme: " + ZawgyiWord.printMM(currWord));
				System.exit(1);
			}
			
			//Ok! Add it up!
			wordsToCheck.add(currWord);
			wordToRhyme.put(currWord, currRhyme);
			if (!rhymeToWords.containsKey(currRhyme))
				rhymeToWords.put(currRhyme, new ArrayList<String>());
			rhymeToWords.get(currRhyme).add(currWord);
		
		}
		
		
		int totalInstances = 0;
		int instancesCaptured = 0;
		for (String s : wordsToCheck) {
			if (!dictionaryCounts.containsKey(s))
				continue;
			totalInstances += dictionaryCounts.get(s)[0];
		}
		
		int totalWords = wordsToCheck.size();
		int totalCaptured = 0;
		ArrayList<String> necessaryRhmes = new ArrayList<String>();
		int currWordID = 0;
		while (((float)instancesCaptured)/totalInstances<0.9F && currWordID<wordsToCheck.size()) {
			String currWord = wordsToCheck.get(currWordID++);
			String currRhyme = wordToRhyme.get(currWord);
			
			//Skip it?
			if (necessaryRhmes.contains(currRhyme))
				continue;
			
			//Add it!
			necessaryRhmes.add(currRhyme);
			totalCaptured += rhymeToWords.get(currRhyme).size();
			
			System.out.println(necessaryRhmes.size() + ":" + rhymeToWords.get(currRhyme));
			
			//Count up
			for (String s : rhymeToWords.get(currRhyme)) {
				if (dictionaryCounts.containsKey(s)) {
					instancesCaptured += dictionaryCounts.get(s)[0];
				}
			}
		}
		
		System.out.println("-------------------------------------");
		System.out.println("Total rhymes: " + rhymeToWords.size());
		System.out.println("Rhymes needed: " + necessaryRhmes.size());
		System.out.println("Words: " + totalCaptured + " of " + totalWords);
		System.out.println("Instances: " + instancesCaptured + " of " + totalInstances);
		
		BufferedWriter rhymeOutfile = null;
		try {
			rhymeOutfile = new BufferedWriter(new PrintWriter("rhymelist.top90.txt", "UTF-8"));
		} catch (FileNotFoundException ex) {
			System.out.println("Out file not found.");
		} catch (UnsupportedEncodingException ex) {
			System.out.println("Out file encoding not supported (UTF-8).");
		}
	
		for (String s : necessaryRhmes) {
			try {
				rhymeOutfile.write(s + "\n");
			} catch (IOException ex) {
				System.out.println("Error writing: " + ex.toString());
			}
		}
		
		try {
			rhymeOutfile.close();
		} catch (IOException ex) {
			System.out.println("Error closing file: " + ex.toString());
		}
	}
	
	
	private void readFiles(String[] args) {
		//Read all onsets...
		String onsetFile = "onsets.zg.txt";
		if (args.length > 0) {
			onsetFile = args[0];
		}
		readLines(onsetFile, new Action() {
			public void perform(String line) {
				Matcher m = Combiner.equalsPattern.matcher(line);
				if (!m.matches()) {
					throw new RuntimeException("Line doesn't match: " + ZawgyiWord.printMM(line));
				}
				
				String mm = m.group(1).trim();
				onsets.put(mm, m.group(2).trim());
			};
		});
		
		
		String rhymeFile = "rhymes.zg.txt";
		if (args.length > 1) {
			rhymeFile = args[1];
		}
		readLines(rhymeFile, new Action() {
			public void perform(String line) {
				Matcher m = Combiner.equalsPattern.matcher(line);
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
				Matcher m = Combiner.equalsPattern.matcher(line);
				if (!m.matches()) {
					throw new RuntimeException("Line doesn't match: " + ZawgyiWord.printMM(line));
				}
				
				String toPut = m.group(1).replaceAll("\\$", "").trim();
				specials.put(toPut, m.group(2).trim());
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
		
		String corpusFile = "data/Corpus.combined.txt";
		if (args.length > 4) {
			wordListFile = args[4];
		}
		readLines(corpusFile, new Action() {
			public void perform(String line) {
				String[] words = line.split("-");
				for (String word : words) {
					if (!dictionaryCounts.containsKey(word))
						dictionaryCounts.put(word, new int[]{0});
					dictionaryCounts.get(word)[0]++;
				}
			};
		});
		
		//Sort the word counts
		ArrayList<Integer> counts = new ArrayList<Integer>();
		Hashtable<Integer, ArrayList<String>> countBack = new Hashtable<Integer, ArrayList<String>>(); 
		for (String k : dictionaryCounts.keySet()) {
			int v = dictionaryCounts.get(k)[0];
			if (!countBack.containsKey(v)) {
				counts.add(v);
				countBack.put(v, new ArrayList<String>());
			}
			countBack.get(v).add(k);
		}
		Collections.sort(counts);
		Collections.reverse(counts);
		
		for (int x : counts) {
			for (String s : countBack.get(x))
				allWordsSortedLargestFirst.add(s);
		}
		
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
