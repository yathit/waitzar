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

import javax.swing.JTable.PrintMode;


public class Combiner {
	
	public static HashMap<String, String> onsets = new HashMap<String, String>();
	public static HashMap<String, String> rhymes = new HashMap<String, String>();
	public static HashMap<String, String> specials = new HashMap<String, String>();
	
	public static HashMap<String, ArrayList<String>> wordsPerRhyme = new HashMap<String, ArrayList<String>>();
	
	public static ArrayList<ZawgyiWord> dictionary = new ArrayList<ZawgyiWord>();
	public static ArrayList<String> dictionaryUnsorted = new ArrayList<String>();
	public static ArrayList<String> incorrectWords = new ArrayList<String>();
	
	public static ArrayList<ZawgyiWord> firstGenerationDictionary = new ArrayList<ZawgyiWord>();
	public static ArrayList<String> firstGenerationUnsorted = new ArrayList<String>();
	
	public static Pattern equalsPattern = Pattern.compile("([^=]+)=(.+)");
	
	private static int errorCount;
	
	
	//DEBUG:
	private static ArrayList<String> lastOnsetMappings = new ArrayList<String>();
	private static String lastOnset = "";
	

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
		StringBuilder afterDigitsBuffer = new StringBuilder();
		for (String word : dictionaryUnsorted) {
			//Special cases:
			if (specials.containsKey(word)) {
				try {
					String line = word + " = " + specials.get(word) + "\n";
					if (word.length()==1 && word.charAt(0)>=0x1040 && word.charAt(0)<=0x1049) {
							writeFile.write(line);
					} else
						afterDigitsBuffer.append(line);
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
			
			//DEBUG: track?
			if (onset.equals(lastOnset)) {
				lastOnsetMappings.add(word);
			}
			
			//Special case: "sh" words might re-order the rhyme in order to match the onset.
			String rhyme = word.replaceFirst(onset, "-");
			if (onset.length()>0 && onsets.get(onset).equals("sh")) {
				rhyme = rhyme.replaceAll("\u1033\u102D", "\u102D\u1033");
			}
			
			//Get the rhyme
			if (!rhymes.containsKey(rhyme)){
				incorrectWords.add(rhyme);
				wordsPerRhyme.put(rhyme, new ArrayList<String>());
				//throw new RuntimeException("["+count+"/"+dictionaryUnsorted.size()+"] Cannot find word for: " + ZawgyiWord.printMM(onset) + " + " + ZawgyiWord.printMM(rhyme));
			}
			wordsPerRhyme.get(rhyme).add(word);
			
			//Print this option in our dictionary
			String onsetStr = onsets.get(onset);
			if (onset.length()==1 && onset.charAt(0)=='\u1021')
				onsetStr = "";
			afterDigitsBuffer.append(word + " = " + onsetStr + rhymes.get(rhyme) + "\n");
			
			count++;
		}
		
		try {
			writeFile.write(afterDigitsBuffer.toString());
		} catch (IOException ex) {
			System.out.println("Error writing: " + ex.toString());
			System.exit(1);
		}
		
		
		
		//Also check that we have all words
		System.out.println("Checking for anomalies, please wait...");
		StringBuffer sb = new StringBuffer();
		for (ZawgyiWord word : firstGenerationDictionary) {
			boolean foundExactMatch = false;
			boolean foundCanonicalMatch = false;
			String matchFound = "";
			for (ZawgyiWord test : dictionary) {
				if (word.toString().equals(test.toString())) {
					matchFound = test.toString();
					foundExactMatch = true;
					break;
				} else if (word.toCanonString().equals(test.toCanonString())) {
					matchFound = test.toString();
					foundCanonicalMatch = true;
				}
			}
			
			if (!foundExactMatch) {
				if (foundCanonicalMatch) {
					sb.append("\n" + word + " canon to: " + matchFound);
				} else {
					sb.append("\n" + word + "  missing!");
				}
			}
		}
		
		if (sb.length()>0) {
			try {
				writeFile.write("\n\nANOMALIES:" + sb.toString());
			} catch (IOException ex) {
				System.out.println("Error: " + ex.toString());
			}
		}
		
		
		
		//Now, order and output our rhymes.
		BufferedWriter rhymeOutfile = null;
		try {
			rhymeOutfile = new BufferedWriter(new PrintWriter("rhymelist.generated.txt", "UTF-8"));
		} catch (FileNotFoundException ex) {
			System.out.println("Out file not found.");
		} catch (UnsupportedEncodingException ex) {
			System.out.println("Out file encoding not supported (UTF-8).");
		}
		
		
		ArrayList<ZawgyiWord> rhymesOrdered = new ArrayList<ZawgyiWord>();
		for (String s : rhymes.keySet()) {
			try {
				ZawgyiWord z = new ZawgyiWord(s);
				rhymesOrdered.add(z);
			} catch (RuntimeException r) {
				if (r.getMessage().startsWith("Final already set")) {
					incorrectWords.add(s);
				} else
					throw new RuntimeException(r);
			}
		}
		Collections.sort(rhymesOrdered, ZawgyiWord.getComparator());
		ZawgyiWord pastWord = null;
		sb = new StringBuffer();
		for (ZawgyiWord zg : rhymesOrdered) {
			if (pastWord==null || !zg.toCanonString().equals(pastWord.toCanonString())) {
				sb.append("\n");
			}
			
			sb.append(zg.toString() + " = " + rhymes.get(zg.toString()) + "\t");
			String sep = "";
			for (String possWord : wordsPerRhyme.get(zg.toString())) {
				sb.append(sep).append(possWord);
				sep = ", ";
			}
			if (wordsPerRhyme.get(zg.toString()).size()==0)
				sb.append("NO_WORDS");
			sb.append("\n");
			
			pastWord = zg;
			try {
				rhymeOutfile.write(sb.toString());
			} catch (IOException ex) {
				System.out.println("Error: " + ex.toString());
				return;
			}
			sb = new StringBuffer();
		}
		
		//Add:
		sb.append("\nNOT SORTED:\n");
		for (String s : incorrectWords) {
			sb.append(s + " = " + rhymes.get(s) + "\t");
			String sep = "";
			for (String possWord : wordsPerRhyme.get(s)) {
				sb.append(sep).append(possWord);
				sep = ", ";
			}
			sb.append("\n");
		}
		try {
			rhymeOutfile.write(sb.toString());
		} catch (IOException ex) {
			System.out.println("Error: " + ex.toString());
			return;
		}
		
		//DEBUG: Output
		String sep = "";
		try {
			writeFile.write("\n\nLAST ONSET:" + lastOnset + "\n  [");
			for (String word : lastOnsetMappings) {
				writeFile.write(sep + word);
				sep = ", ";
			}
			writeFile.write("]\n");
		} catch (IOException ex) {
			System.out.println("can't write");
			ex.printStackTrace();
		}
		
		
		try {
			writeFile.close();
			rhymeOutfile.close();
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
				
				String mm = m.group(1).trim();
				onsets.put(mm, m.group(2).trim());
				lastOnset = mm;
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
				
				if (wordsPerRhyme.containsKey(m.group(1).trim())) {
					throw new RuntimeException("Word already in dictionary: " + ZawgyiWord.printMM(m.group(1)));
				}
				wordsPerRhyme.put(m.group(1).trim(), new ArrayList<String>());
			};
		});
		

		String specialFile = "specials.zg.txt";
		if (args.length > 2) {
			specialFile = args[2];
		}
		errorCount = 0;
		readLines(specialFile, new Action() {
			public void perform(String line) {
				Matcher m = equalsPattern.matcher(line);
				if (!m.matches()) {
					throw new RuntimeException("Line doesn't match: " + ZawgyiWord.printMM(line));
				}
				
				String toPut = m.group(1).replaceAll("\\$", "").trim();
				if (specials.containsKey(toPut)) {
					System.out.println("Error! Already contains special word: " + ZawgyiWord.printMM(toPut) + " as " + specials.get(toPut));
					errorCount++;
				}
				specials.put(toPut, m.group(2).trim());
			};
		});
		if (errorCount>0)
			throw new RuntimeException("Too many errors (see above).");
		

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
		
		
		String firstGenWordListFile = "MyanmarList_v1.txt";
		if (args.length > 4) {
			wordListFile = args[4];
		}
		readLines(firstGenWordListFile, new Action() {
			public void perform(String lineTxt) {
				String zg = lineTxt.split("=")[0].trim();
				
				try {
					firstGenerationDictionary.add(new ZawgyiWord(zg.trim()));
				} catch (RuntimeException ex) {
					firstGenerationUnsorted.add(zg.trim());
				}
			};
		});
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
