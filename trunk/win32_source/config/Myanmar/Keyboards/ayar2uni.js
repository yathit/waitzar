//Avoid complicated processing for single letters
if (val.length==1)
	return val

//Burmese consonant pattern is (for now):
//  [\u1000-\u102A,\u103F,\u1040-\u1049,\u104E]

//Letters in between the consonant and kinzi may include stacked letters, the
//  prefix medials that we just re-arranged, or a few other scattered medials:
//  (?:[\u1031\u103C\u103B\u102D\u102E\u1032\u1030\u1031\u1036\u1037\u103D\u103E]|(?:\u1039[\u1000-\u102A,\u103F,\u1040-\u1049,\u104E]))*

//Step 1: catch U+1031 U+103C CONS; this pattern is quite rigid
// (no-one expects to insert anything in between these characters).
val = val.replace(/(\u1031?)(\u103C?)([\u1000-\u102A,\u103F,\u1040-\u1049,\u104E])/g, '$3$2$1');


//Step 2: catch CONS + kinzi; this pattern is fluid. Not only can
// our previous search put vowels in between, but other letters might
// be put there too
val = val.replace(/([\u1000-\u102A,\u103F,\u1040-\u1049,\u104E])((?:[\u1031\u103C\u103B\u102D\u102E\u1032\u1030\u1031\u1036\u1037\u103D\u103E]|(?:\u1039[\u1000-\u102A,\u103F,\u1040-\u1049,\u104E]))*)(\u1004\u103A\u1039)/g, '$3$1$2');


//Step 3: Fix normalization error introduced by the first step
val = val.replace(/([\u1000-\u102A,\u103F,\u1040-\u1049,\u104E])(\u103C?)(\u1031?)((?:\u1039[\u1000-\u102A,\u103F,\u1040-\u1049,\u104E])?)(\u103B?)(\u103D?)(\u103E?)/g, '$1$4$5$2$6$7$3');
