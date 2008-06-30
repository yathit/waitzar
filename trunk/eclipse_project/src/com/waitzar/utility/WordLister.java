package com.waitzar.utility;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;


/**
 * The main method of this class converts the Myanmar.model file to Myanmar.model.txt encoded
 *  as UTF-8. Order is preserved, so that one can use FontProfileCompiler to convert BACK,
 *  for use as a font in Wait Zar.
 * @author Seth N. Hetu
 */
public class WordLister {

	/**
	 * @param args [0] = path to file... assume "Myanmar.model" otherwise. [1] = output file... assume path+".txt" otherwise
	 */
	public static void main(String[] args) {
		String inputFileName = "Myanmar.model";
		if (args.length>0)
			inputFileName = args[0];
		
		String outputFileName = inputFileName + ".txt";
		if (args.length>1)
			outputFileName = args[1];
		
		//Do these files exist?
		BufferedReader inFile = null;
		try {
			inFile = new BufferedReader(new InputStreamReader(new FileInputStream(inputFileName)));
		} catch (FileNotFoundException ex) {
			System.out.println("Model file not found: " + inputFileName);
			return;
		}
		
		BufferedWriter writeFile = null;
		try {
			writeFile = new BufferedWriter(new PrintWriter(outputFileName, "UTF-8"));
		} catch (FileNotFoundException ex) {
			System.out.println("Cannot output to file: " + outputFileName);
		} catch (UnsupportedEncodingException ex) {
			System.out.println("Out file encoding not supported (UTF-8).");
		}
		
		if (inFile!=null && writeFile!=null) {
			convert(inFile, writeFile);
		}
		
		try {
			inFile.close();
			writeFile.close();
		} catch (IOException ex) {
			
		}
		
		System.out.println("Done");
	}
	
	
	private static void convert(BufferedReader inFile, BufferedWriter outFile) {
		//Get our definitions & write them
		//ArrayList<String> words = new ArrayList<String>();
		try {
			boolean oneComment = false;
			for (String line = inFile.readLine(); line!=null; line=inFile.readLine()) {
				//Comments
				line = line.trim();
				if (line.length()==0 || line.startsWith("#")) {
					if (oneComment)
						break;
					
					oneComment = true;
					continue;
				}
				line = line.replaceFirst("[0-9]+\\[", "");
				System.out.println("Line: " + line);
				
				//Get...
				StringBuilder currWord = new StringBuilder();
				for (int i=0; i<line.length(); i++) {
					char c = line.charAt(i);
					if (c==',' || c==']') {
						outFile.write(currWord.append("\n").toString());
						currWord = new StringBuilder();
					} else if (c=='-') {
					} else {
						//Add the next two...
						currWord.append(""+(char)(0x1000+Integer.parseInt(c + "" + line.charAt(++i), 16)));
					}
				}
			}
		} catch (IOException ex) {
			System.out.println("Error reading: " + ex.toString());
		}	
	}

}


