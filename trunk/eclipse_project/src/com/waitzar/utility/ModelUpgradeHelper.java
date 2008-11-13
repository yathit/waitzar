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
import java.util.Hashtable;

import com.waitzar.performance.wrapper.Myanmar;
import com.waitzar.performance.wrapper.Roman;


/**
 * In order to help upgrade from the 1.0 to the 2.0 model, this class will take both 
 *  and list which words are different (or added) for the 2.0 model. 
 * @author Seth N. Hetu
 */
public class ModelUpgradeHelper {

	/**
	 * @param args [model1Path, model2Path]
	 */
	public static void main(String[] args) {
		String model1Path = "MyanmarList_v1.txt";
		String model2Path = "MyanmarList_v2.txt";
		if (args.length>0) {
			model1Path = args[0];
			if (args.length>1)
				model2Path = args[1];
		}
		
		//Open the input files
		Hashtable<String, String> oldModel = new Hashtable<String, String>();
		Hashtable<String, String> newModel = new Hashtable<String, String>();
		Hashtable<String, ArrayList<String>> newModelPairs = new Hashtable<String, ArrayList<String>>();
		loadModel(model1Path, oldModel, null);
		loadModel(model2Path, newModel, newModelPairs);
		
		System.out.println("Processing files...");
		
		//Open the output file
		BufferedWriter outFile = null;
		String outputFileName = "MyanmarList_differences.txt";
		try {
			outFile = new BufferedWriter(new PrintWriter(outputFileName, "UTF-8"));
		} catch (FileNotFoundException ex) {
			System.out.println("Cannot output to file: " + outputFileName);
		} catch (UnsupportedEncodingException ex) {
			System.out.println("Out file encoding not supported (UTF-8).");
		}
		
		//Header
		try {
			outFile.write("Myanmar" + "\t" + "Old" + "\t" + "New" + "\n");
		} catch (IOException ex) {
			System.out.println("Error writing to file!");
			ex.printStackTrace();
		}
		
		//Write each pair
		for (String myanmar : newModel.keySet()) {
			String oldRoman = oldModel.get(myanmar);
			if (oldRoman==null)
				oldRoman = "";
			String newRoman = newModel.get(myanmar);
			if (newRoman.equals(oldRoman))
				continue;
			
			try {
				outFile.write(myanmar + "\t" + oldRoman + "\t" + newRoman + "\n");
			} catch (IOException ex) {}
		}
		
		//Done
		try {
			outFile.close();
		} catch (IOException ex) {
			ex.printStackTrace();
		}
		
		//Print something useful....
		int maxPairs = 0;
		for (String s : newModelPairs.keySet()) {
			int count = newModelPairs.get(s).size();
			if (count>maxPairs)
				maxPairs = count;
		}
		
		String sep = "";
		System.out.println("Maximum Pairs: " + maxPairs);
		System.out.print("Found In: ");
		for (String s : newModelPairs.keySet()) {
			int count = newModelPairs.get(s).size();
			if (count==maxPairs) {
				System.out.print(sep + s);
				sep = ", ";
			}
		}
		System.out.println();
		
		System.out.println("Done");
	}

	
	public static void loadModel(String fileName, Hashtable<String, String> dictionary, Hashtable<String, ArrayList<String>> modelPairs) {
		//Open the model file
		BufferedReader reader = null;
		try {
			reader = new BufferedReader(new InputStreamReader(new FileInputStream(fileName), "UTF-8"));
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		
		//Read each line
		String line = null;
		try {
			while ((line = reader.readLine()) != null) {
				// Comments, empty lines
				line = line.trim();
				if (line.length() == 0 || line.charAt(0) == '#')
					continue;

				// Assign a word to our dictionary
				String[] halves = line.split("=");
				String mm = halves[0].trim();
				String roman = halves[1].trim();
				dictionary.put(mm, roman);
				
				if (modelPairs!=null) {
					if (!modelPairs.containsKey(roman))
						modelPairs.put(roman, new ArrayList<String>());
					modelPairs.get(roman).add(mm);
				}
			}
		} catch (IOException ex) {
			ex.printStackTrace();
		}
		
		try {
			reader.close();
		} catch (IOException ex) {
			ex.printStackTrace();
		}
	}	
}
