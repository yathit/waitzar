package com.waitzar.wordsearch;

public class Runner {
	
	
	
	
	//Build up our model from the rhymelist, then tell how many rhymes are needed for 90% coverage.
	//  Sort our words by their total occurrences in the corpus.
	public static void main(String[] args) {
		new WordSearcher(args);
	}
}
