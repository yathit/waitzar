package com.waitzar.performance;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Date;
import java.util.Hashtable;

import com.waitzar.analysis.ZawgyiWord;
import com.waitzar.performance.wrapper.*;

public class LanguageModel {	
	//Dictionary of Myanmar = Roman mappings
	private Hashtable<Myanmar, Roman> dictionary;
	
	//The trigrams listed for statistical use (e.g., perplexity) are of the form:
	//  <MYi-2, MYi-1, MYi> -> count
	//  ...where either of the MYi<0 can be <BOS> and MYi should be <EOS> once for each sentence.
	//Note that we also need some data on bigrams & prefixes. In particular, we need:
	//  <MYi-2, MYi-1> --> gamma
	//  <MYi-1, MYi> --> pSmooth
	private Hashtable<String, Trigram> trigramCounts;
	private long totalOfTrigramCounts;
	private Hashtable<String, Bigram> gammaValues;
	private Hashtable<String, Bigram> pSmoothPrevValues;
	
	
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
		trigramCounts = new Hashtable<String, Trigram>();
		totalOfTrigramCounts = 0;
		gammaValues = new Hashtable<String, Bigram>();
		pSmoothPrevValues = new Hashtable<String, Bigram>();
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
						if (!trigramCounts.containsKey(t.toString())) {
							trigramCounts.put(t.toString(), t);
						}
						trigramCounts.get(t.toString()).numberOfOccurrences++;
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
	
	
	
	//Based off the test data
	public double calculatePerplexity() {
		//Compute the probability of this test set.
		double pTestSet = 1.0;
		long numberOfWordsIncEOS = 0;
		for (Myanmar[] sentence : testData) {
			//Init
			Myanmar penultimateWord = new Myanmar(Myanmar.BOS);
			Myanmar ultimateWord = new Myanmar(Myanmar.BOS);
			Myanmar word = null;
			
			//Compute the probability of this sentence
			double pSentence = 1.0;
			for (int i=0; i<=sentence.length; i++) {
				//Get the word (or <EOS>)
				if (i<sentence.length)
					word = sentence[i];
				else
					word = new Myanmar(Myanmar.EOS);
				numberOfWordsIncEOS++;
			
				//Test... increase perplexity
				double conditionalProbability = getSmoothedProbability(new Trigram(penultimateWord, ultimateWord, word));
				//System.out.println("  probability: " + conditionalProbability);
				pSentence *= conditionalProbability;
			
				//Increment
				penultimateWord = new Myanmar(ultimateWord);
				ultimateWord = new Myanmar(word);
			}
			
			//Accumulate with the probability for this test set.
			pTestSet *= pSentence;
		}
		
		//Now that we have the probability of this test set, compute the cross-entropy.
		double hPtest = (-1.0/numberOfWordsIncEOS) * (Math.log(pTestSet)/Math.log(2.0));
		
		//Finally, we get the per-word perplexity
		double ppWTest = Math.pow(2.0, hPtest);
		return ppWTest;
	}
	
	
	
	//Must be called each time a change is made to the model
	public void smoothModel() {
		//First, we need to smooth the model
		totalOfTrigramCounts = 0;
		
		//Reset
		gammaValues.clear();
		pSmoothPrevValues.clear();
		for (String key : trigramCounts.keySet()) {
			Trigram tri = trigramCounts.get(key);
			tri.halfOfAlphaComponent = 0.0;
			totalOfTrigramCounts += tri.numberOfOccurrences;
			
			//Add the relevant bigrams if they're not already contained...
			Bigram prefix = new Bigram(tri.getPenultimateWord(), tri.getUltimateWord());
			if (!gammaValues.containsKey(prefix.toString()))
				gammaValues.put(prefix.toString(), prefix);
			Bigram biGram = new Bigram(tri.getUltimateWord(), tri.getWord());
			if (!pSmoothPrevValues.containsKey(biGram.toString()))
				pSmoothPrevValues.put(biGram.toString(), biGram);
			
			if (tri.numberOfOccurrences==0)
				throw new RuntimeException("Invalid: Zero trigram occurrences!");
		}
		
		//First, compute the D-values (saves time later)
		double D1 = 0.0;
		double D2 = 0.0;
		double D3plus = 0.0;
		if (true) {// Scope
			// Count our Ns
			long n1 = 0;
			long n2 = 0;
			long n3 = 0;
			long n4 = 0;
			for (String key : trigramCounts.keySet()) {
				Trigram candidate = trigramCounts.get(key);
				
				if (candidate.numberOfOccurrences > Integer.MAX_VALUE)
					throw new RuntimeException("Too many counts: " + candidate.numberOfOccurrences);
				
				switch ((int)candidate.numberOfOccurrences) {
				case 1:
					n1++;
					break;
				case 2:
					n2++;
					break;
				case 3:
					n3++;
					break;
				case 4:
					n4++;
					break;
				}
			}

			// Determine our possible d-values
			double Y = n1 / (n1 + 2.0 * n2);
			D1 = 1.0 - (2.0 * Y * n2) / n1;
			D2 = 2.0 - (3.0 * Y * n3) / n2;
			D3plus = 3.0 - (4.0 * Y * n4) / n3;
		}
		
		//Now, for each trigram...
		Date d = new Date();
		System.out.println("Started training of trigrams("+trigramCounts.keySet().size()+") at " + d.getHours() + ":" + d.getMinutes());
		for (String key : trigramCounts.keySet()) {
			//Init
			Trigram tri = trigramCounts.get(key);
			
			//Calculate: d-value
			double dValue = 0.0;
			if (true) {//Scope
				//Set the D-value appropriately.
				switch ((int)tri.numberOfOccurrences) {
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
			}
			
			
			//Calculate: alpha component
			if (true) {//Scope
				//Numerator
				double alphaTri = tri.numberOfOccurrences - dValue;
				if (alphaTri<0.0)
					alphaTri = 0.0;
				
				//Denominator
				alphaTri /= (totalOfTrigramCounts);
				
				//Discount later
				tri.halfOfAlphaComponent = alphaTri;
			}
		}
		
		
		//Now, for each of the gamma bigrams
		d = new Date();
		System.out.println("Started training of gamma Bigrams("+gammaValues.keySet().size()+") at " + d.getHours() + ":" + d.getMinutes());
		for (String key : gammaValues.keySet()) {
			Bigram big = gammaValues.get(key);
			
			//Compute the numerator. 
			double countN1 = 0.0;
			double countN2 = 0.0;
			double countN3plus = 0.0;
			boolean[] matcher = new boolean[]{true, true, false};
			Trigram tri =  new Trigram(big.getUltimateWord(), big.getWord(), null);
			for (String key2 : trigramCounts.keySet()) {
				Trigram candidate = trigramCounts.get(key2);
				
				if (tri.equals(candidate, matcher)) {
					switch ((int)candidate.numberOfOccurrences) {
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
			double D1comp = D1*countN1;
			double D2comp = D2*countN2;
			double D3plus_comp = D3plus*countN3plus;
			double numerator = D1comp + D2comp + D3plus_comp;
			
			//Divide by the denominator, store
			big.doubleComponent = numerator/totalOfTrigramCounts;
		}
		
		
		//Now, for each of the pSmoothPrev values
		d = new Date();
		System.out.println("Started training of smoothed Bigrams("+pSmoothPrevValues.keySet().size()+") at " + d.getHours() + ":" + d.getMinutes());
		for (String key3 : pSmoothPrevValues.keySet()) {
			Bigram big = pSmoothPrevValues.get(key3);
			
			double pSmoothNumerator = 0.0;
			long pSmoothDenominator = 0;
			boolean[] check = new boolean[]{false, true, true};
			boolean[] check2 = new boolean[]{false, true, false};
			Trigram tri = new Trigram(null, big.getUltimateWord(), big.getWord());
			for (String key4 : trigramCounts.keySet()) {
				Trigram candidate = trigramCounts.get(key4);
				
				if (tri.equals(candidate, check))
					pSmoothNumerator++;
				if (tri.equals(candidate, check2))
					pSmoothDenominator++;
			}
			big.doubleComponent = pSmoothNumerator/pSmoothDenominator;
			
			//Both are zero:
			System.out.println("test: " + pSmoothNumerator + " / " + pSmoothDenominator);
		}
		
		
		//Done
		d = new Date();
		System.out.println("Finished smoothing at " + d.getHours() + ":" + d.getMinutes());
	}
	
	
	public double getSmoothedProbability(Trigram t) {
		//Compute the tail-end component; we'll need it regardless of our method.
		Bigram gammaComp = gammaValues.get(new Bigram(t.getPenultimateWord(), t.getUltimateWord()).toString());
		Bigram pPrevComp = pSmoothPrevValues.get(new Bigram(t.getUltimateWord(), t.getWord()).toString());
		double gamma = (gammaComp==null) ? 0.0 : gammaComp.doubleComponent;
		double pSmoothPrev = (pPrevComp==null) ? 0.0 : pPrevComp.doubleComponent;
		double dotDotDot = gamma * pSmoothPrev;
		
		//This is all, if there's no alpha component.
		Trigram tD = trigramCounts.get(t.toString());
		if (tD == null) {
		//	System.out.println("  probability: " + gamma + " * " + pSmoothPrev);
			return dotDotDot;
		}
		
		//System.out.println("  probability: " + tD.halfOfAlphaComponent + " + " + gamma + " * " + pSmoothPrev);
		
		//Else... figure out the alpha component and add it
		return tD.halfOfAlphaComponent + dotDotDot;
	}
	
	

}



