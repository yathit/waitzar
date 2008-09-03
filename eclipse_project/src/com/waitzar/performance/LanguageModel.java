package com.waitzar.performance;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Hashtable;

import com.waitzar.performance.wrapper.*;

public class LanguageModel {	
	//Dictionary of Myanmar = Roman mappings
	private Hashtable<Myanmar, Roman> dictionary;
	
	//The trigrams listed for statistical use (e.g., perplexity) are of the form:
	//  <MYi-2, MYi-1, MYi> -> count
	//  ...where either of the MYi<0 can be <BOS> and MYi should be <EOS> once for each sentence.
	private Hashtable<Trigram, TrigramData> trigramCounts;
	private int totalOfTrigramCounts;
	
	//The "trigrams" used for our estimation (e.g., typing in WaitZar) are of the form:
	//  <MYi-2, MY-i1, RMi> -> <MYi(1) : count, MYi(2) : count, ...>
	// ...the <EOS> and <BOS> tokens make no sense for this model; instead, appeals are simply
	// made to a lower-order model until a result is found.
	
	
	//We only cache the testing data, not the training data
	private ArrayList<Myanmar[]> testData;
	
	
	public LanguageModel(File textModel, File textCorpus) { 
		//Read our model into the dictionary
		readModel(textModel);
		
		//Read our corpus and train our initial model
		readAndTrainCorpus(textCorpus);
	}
	
	
	
	
	
	
	
	private void readModel(File textModel) {
		//Open the model file
		BufferedReader reader = null;
		try {
			reader = new BufferedReader(new InputStreamReader(new FileInputStream(textModel), "UTF-8"));
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		
		//Read each line
		dictionary = new Hashtable<Myanmar, Roman>();
		String line = null;
		try {
			while ((line = reader.readLine()) != null) {
				// Comments, empty lines
				line = line.trim();
				if (line.length() == 0 || line.charAt(0) == '#')
					continue;

				// Assign a word to our dictionary
				String[] halves = line.split("=");
				dictionary.put(new Myanmar(halves[0].trim()), new Roman(halves[1].trim()));
			}
		} catch (IOException ex) {
			ex.printStackTrace();
		}
	}
	
	
	private void readAndTrainCorpus(File textCorpus) {
		//Open the model file
		BufferedReader reader = null;
		try {
			reader = new BufferedReader(new InputStreamReader(new FileInputStream(textCorpus), "UTF-8"));
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		
		//Read each line
		testData = new ArrayList<Myanmar[]>();
		trigramCounts = new Hashtable<Trigram, TrigramData>();
		totalOfTrigramCounts = 0;
		String line = null;
		try {
			int count=0;
			
			while ((line = reader.readLine()) != null) {
				// Comments, empty lines
				line = line.trim();
				if (line.length() == 0 || line.charAt(0) == '#')
					continue;
				
				//Get this line
				Myanmar[] words = Myanmar.createArray(line.split("-"));

				//Assign to our testing set?
				if (count<3)
					count++;
				else {
					count = 0;
					testData.add(words);
					continue;
				}
				
				//If we didn't continue, we should train our model on this data point.
				//Train the classical model:
				if (true) { //Scope
					Myanmar penultimateWord = new Myanmar(Myanmar.BOS);
					Myanmar ultimateWord = new Myanmar(Myanmar.BOS);
					Myanmar word = null;
					for (int i=0; i<=words.length; i++) {
						//Get the word (or <EOS>)
						if (i<words.length)
							word = words[i];
						else
							word = new Myanmar(Myanmar.EOS);
					
						//Train the word
						Trigram t = new Trigram(penultimateWord, ultimateWord, word);
						if (!trigramCounts.containsKey(t))
							trigramCounts.put(t, new TrigramData(0));
						trigramCounts.get(t).numberOfOccurrences++;
						totalOfTrigramCounts++;
					
						//Increment
						penultimateWord = new Myanmar(ultimateWord);
						ultimateWord = new Myanmar(word);
					}
				}
				
				
				//Train out model
				//(later)
				
				
			}
		} catch (IOException ex) {
			ex.printStackTrace();
		}
	}
	
	
	public void computePerplexity() {
		//First, we need to smooth the model
		totalOfTrigramCounts = 0;
		
		//Reset
		for (Trigram tri : trigramCounts.keySet()) {
			TrigramData triData = trigramCounts.get(tri);
			triData.probability = 0.0;
			totalOfTrigramCounts += triData.numberOfOccurrences;
			
			if (triData.numberOfOccurrences==0)
				throw new RuntimeException("Invalid: Zero trigram occurrences!");
		}
		
		//Now, for each trigram...
		for (Trigram tri : trigramCounts.keySet()) {
			//Init
			TrigramData triData = trigramCounts.get(tri);
			double alphaTri = 0.0;
			double gammaTri = 0.0;
			double pSmoothPrev = 0.0;
			double dValue = 0.0;
			
			//Intermediate values
			double D1comp = 0.0;
			double D2comp = 0.0;
			double D3plus_comp = 0.0;
			
			//Calculate: pSmoothPrev
			if (true) {//Scope
				double pSmoothNumerator = 0.0;
				long pSmoothDenominator = 0;
				boolean[] check = new boolean[]{false, true, true};
				boolean[] check2 = new boolean[]{false, true, false};
				for (Trigram candidate : trigramCounts.keySet()) {
					if (tri.equals(candidate, check))
						pSmoothNumerator++;
					if (tri.equals(candidate, check2))
						pSmoothDenominator++;
				}
				pSmoothPrev = pSmoothNumerator/pSmoothDenominator;
			}
			
			//Calculate: d-value
			if (true) {//Scope
				//Count our Ns
				long n1 = 0;
				long n2 = 0;
				long n3 = 0;
				long n4 = 0;
				for (Trigram candidate : trigramCounts.keySet()) {
					switch (trigramCounts.get(candidate).numberOfOccurrences) {
						case 1:
							n1++; break;
						case 2:
							n2++; break;
						case 3:
							n3++; break;
						case 4:
							n4++; break;
					}
				}
				
				//Determine our possible d-values
				double Y = n1/(n1 + 2.0*n2);
				double D1 = 1.0 - (2.0*Y*n2)/n1;
				double D2 = 2.0 - (3.0*Y*n3)/n2;
				double D3plus = 3.0 - (4.0*Y*n4)/n3;
				switch (triData.numberOfOccurrences) {
					case 1:
						dValue = D1;
						break;
					case 2:
						dValue = D2;
						break;
					default: //3+
						dValue = D3plus; 
						break;
				}
				
				//We can save on computations here...
				double countN1 = 0.0;
				double countN2 = 0.0;
				double countN3plus = 0.0;
				boolean[] matcher = new boolean[]{true, true, false};
				for (Trigram candidate : trigramCounts.keySet()) {
					if (tri.equals(candidate, matcher)) {
						switch (trigramCounts.get(candidate).numberOfOccurrences) {
							case 1:
								countN1++;
								break;
							case 2:
								countN2++;
								break;
							default:
								countN3plus++;
								break;
						}
					}
				}
				D1comp = D1*countN1;
				D2comp = D2*countN2;
				D3plus_comp = D3plus*countN3plus;
			}
			
			
			//Calculate: alpha value
			if (true) {//Scope
				//Numerator
				alphaTri = triData.numberOfOccurrences - dValue;
				if (alphaTri<0.0)
					alphaTri = 0.0;
				
				//Denominator
				alphaTri /= (totalOfTrigramCounts);
				
				//Discount
				alphaTri += pSmoothPrev;
			}
			
			
			//Calculate: gamma value
			gammaTri = (D1comp * D2comp * D3plus_comp)/totalOfTrigramCounts;
			
		}
		
		
		
	}
	
	

}



