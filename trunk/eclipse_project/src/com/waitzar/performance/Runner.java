package com.waitzar.performance;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
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
		//Create and test a language model
		long start = System.currentTimeMillis();
		LanguageModel lm = new LanguageModel(new File("data/MyanmarList_v1.1.txt"), new File("data/Corpus.combined.txt"));
		System.out.println("Trained in: " + ((System.currentTimeMillis()-start)/100) + "ns");
		start = System.currentTimeMillis();
		lm.smoothModel();
		System.out.println("Smoothed in: " + ((System.currentTimeMillis()-start)/100) + "ns");
		
		//Finally, test the perplexity
		start = System.currentTimeMillis();
		double perplexity = lm.calculatePerplexity();
		System.out.println("Perplexity computed in: " + ((System.currentTimeMillis()-start)/100) + "ns");
		System.out.println("Perplexity value: " + perplexity);
		
		
		System.out.println("Done");
	}

}
