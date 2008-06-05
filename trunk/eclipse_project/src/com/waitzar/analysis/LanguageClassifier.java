package com.waitzar.analysis;

import java.util.*;
import java.util.regex.*;

public class LanguageClassifier {
	public static final String UNDEF = "#UNDEF!";
	public static final String NULL = "#null";
	
	public static char THE_BASE = 0x101E;
	public static char THE_END = 0x103D;
	public static char VOWELL_AA = 0x102C;

	public enum SEMANTICS {
		MY_BASE,
		MY_LEADING,
		MY_TRAILING,
		MY_PARTIAL,
		MY_VIRAMA,
		MY_STOP,
		MY_OTHER,
		MY_PAT_SINT,
		SPACE_OR_PUNCT,
		NOT_MYANMAR
	}
	
	public enum KEYPRESS {
		ZG_ONEKEY,
		ZG_SHIFT,
		ZG_TILDE,
		ZG_SHIFT_TILDE
	}
	
	public static KEYPRESS getZawgyiKeypress(char unicodeChar) {
		switch (unicodeChar) {
		
			//100X
			case 0x1000: return KEYPRESS.ZG_ONEKEY;
			case 0x1001: return KEYPRESS.ZG_ONEKEY;
			case 0x1002: return KEYPRESS.ZG_SHIFT;
			case 0x1003: return KEYPRESS.ZG_SHIFT;
			case 0x1004: return KEYPRESS.ZG_ONEKEY;
			case 0x1005: return KEYPRESS.ZG_ONEKEY;
			case 0x1006: return KEYPRESS.ZG_ONEKEY;
			case 0x1007: return KEYPRESS.ZG_SHIFT;
			case 0x1008: return KEYPRESS.ZG_TILDE;
			case 0x1009: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x100A: return KEYPRESS.ZG_ONEKEY;
			case 0x100B: return KEYPRESS.ZG_SHIFT;
			case 0x100C: return KEYPRESS.ZG_SHIFT;
			case 0x100D: return KEYPRESS.ZG_SHIFT;
			case 0x100E: return KEYPRESS.ZG_TILDE;
			case 0x100F: return KEYPRESS.ZG_SHIFT;
			
			//101X
			case 0x1010: return KEYPRESS.ZG_ONEKEY;
			case 0x1011: return KEYPRESS.ZG_ONEKEY;
			case 0x1012: return KEYPRESS.ZG_ONEKEY;
			case 0x1013: return KEYPRESS.ZG_ONEKEY;
			case 0x1014: return KEYPRESS.ZG_ONEKEY;
			case 0x1015: return KEYPRESS.ZG_ONEKEY;
			case 0x1016: return KEYPRESS.ZG_ONEKEY;
			case 0x1017: return KEYPRESS.ZG_SHIFT;
			case 0x1018: return KEYPRESS.ZG_ONEKEY;
			case 0x1019: return KEYPRESS.ZG_ONEKEY;
			case 0x101A: return KEYPRESS.ZG_ONEKEY;
			case 0x101B: return KEYPRESS.ZG_SHIFT;
			case 0x101C: return KEYPRESS.ZG_ONEKEY;
			case 0x101D: return KEYPRESS.ZG_SHIFT;
			case 0x101E: return KEYPRESS.ZG_ONEKEY;
			case 0x101F: return KEYPRESS.ZG_ONEKEY;
			
			//102X
			case 0x1020: return KEYPRESS.ZG_SHIFT;
			case 0x1021: return KEYPRESS.ZG_ONEKEY;
			case 0x1023: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1024: return KEYPRESS.ZG_TILDE;
			case 0x1025: return KEYPRESS.ZG_SHIFT;
			case 0x1026: return KEYPRESS.ZG_TILDE;
			case 0x1027: return KEYPRESS.ZG_SHIFT;
			//case 0x1029: return KEYPRESS.ZG_; ~5, but seems bad to me...
			case 0x102A: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x102B: return KEYPRESS.ZG_ONEKEY;
			case 0x102C: return KEYPRESS.ZG_ONEKEY;
			case 0x102D: return KEYPRESS.ZG_ONEKEY;
			case 0x102E: return KEYPRESS.ZG_SHIFT;
			case 0x102F: return KEYPRESS.ZG_ONEKEY;
			
			//103X
			case 0x1030: return KEYPRESS.ZG_ONEKEY;
			case 0x1031: return KEYPRESS.ZG_ONEKEY;
			case 0x1032: return KEYPRESS.ZG_SHIFT;
			case 0x1033: return KEYPRESS.ZG_SHIFT;
			case 0x1034: return KEYPRESS.ZG_SHIFT;
			case 0x1036: return KEYPRESS.ZG_SHIFT;
			case 0x1037: return KEYPRESS.ZG_ONEKEY;
			case 0x1038: return KEYPRESS.ZG_ONEKEY;
			case 0x1039: return KEYPRESS.ZG_ONEKEY;
			case 0x103A: return KEYPRESS.ZG_ONEKEY;
			case 0x103B: return KEYPRESS.ZG_ONEKEY;
			case 0x103C: return KEYPRESS.ZG_SHIFT;
			case 0x103D: return KEYPRESS.ZG_SHIFT;
			
			//104X
			case 0x1040: return KEYPRESS.ZG_ONEKEY;
			case 0x1041: return KEYPRESS.ZG_ONEKEY;
			case 0x1042: return KEYPRESS.ZG_ONEKEY;
			case 0x1043: return KEYPRESS.ZG_ONEKEY;
			case 0x1044: return KEYPRESS.ZG_ONEKEY;
			case 0x1045: return KEYPRESS.ZG_ONEKEY;
			case 0x1046: return KEYPRESS.ZG_ONEKEY;
			case 0x1047: return KEYPRESS.ZG_ONEKEY;
			case 0x1048: return KEYPRESS.ZG_ONEKEY;
			case 0x1049: return KEYPRESS.ZG_ONEKEY;
			case 0x104A: return KEYPRESS.ZG_SHIFT;
			case 0x104B: return KEYPRESS.ZG_ONEKEY;
			case 0x104C: return KEYPRESS.ZG_TILDE;
			case 0x104D: return KEYPRESS.ZG_TILDE;
			case 0x104E: return KEYPRESS.ZG_TILDE;
			case 0x104F: return KEYPRESS.ZG_ONEKEY;

			//105X
			case 0x105A: return KEYPRESS.ZG_SHIFT;
			
			//106X
			case 0x1060: return KEYPRESS.ZG_TILDE;
			case 0x1061: return KEYPRESS.ZG_TILDE;
			case 0x1062: return KEYPRESS.ZG_TILDE;
			case 0x1063: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1064: return KEYPRESS.ZG_SHIFT;
			case 0x1065: return KEYPRESS.ZG_TILDE;
			case 0x1066: return KEYPRESS.ZG_TILDE;
			case 0x1067: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1068: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1069: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x106A: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x106B: return KEYPRESS.ZG_TILDE;
			case 0x106C: return KEYPRESS.ZG_TILDE;
			case 0x106D: return KEYPRESS.ZG_TILDE;
			case 0x106E: return KEYPRESS.ZG_TILDE;
			case 0x106F: return KEYPRESS.ZG_SHIFT_TILDE;
			
			//107X
			case 0x1070: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1071: return KEYPRESS.ZG_TILDE;
			case 0x1072: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1073: return KEYPRESS.ZG_TILDE;
			case 0x1074: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1075: return KEYPRESS.ZG_TILDE;
			case 0x1076: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1077: return KEYPRESS.ZG_TILDE;
			case 0x1078: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1079: return KEYPRESS.ZG_TILDE;
			case 0x107A: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x107B: return KEYPRESS.ZG_TILDE;
			case 0x107C: return KEYPRESS.ZG_TILDE;
			case 0x107D: return KEYPRESS.ZG_TILDE;
			case 0x107E: return KEYPRESS.ZG_SHIFT;
			case 0x107F: return KEYPRESS.ZG_SHIFT;
			
			//108X
			case 0x1080: return KEYPRESS.ZG_SHIFT;
			case 0x1081: return KEYPRESS.ZG_TILDE;
			case 0x1082: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1083: return KEYPRESS.ZG_TILDE;
			case 0x1084: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1085: return KEYPRESS.ZG_TILDE;
			case 0x1086: return KEYPRESS.ZG_TILDE;
			case 0x1087: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1088: return KEYPRESS.ZG_SHIFT;
			case 0x1089: return KEYPRESS.ZG_TILDE;
			case 0x108A: return KEYPRESS.ZG_SHIFT;
			case 0x108B: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x108C: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x108D: return KEYPRESS.ZG_TILDE;
			case 0x108E: return KEYPRESS.ZG_TILDE;
			case 0x108F: return KEYPRESS.ZG_SHIFT;
			
			//109X
			case 0x1090: return KEYPRESS.ZG_TILDE;
			case 0x1091: return KEYPRESS.ZG_SHIFT;
			case 0x1092: return KEYPRESS.ZG_SHIFT;
			case 0x1094: return KEYPRESS.ZG_SHIFT;
			case 0x1095: return KEYPRESS.ZG_SHIFT;
			case 0x1096: return KEYPRESS.ZG_SHIFT_TILDE;
			case 0x1097: return KEYPRESS.ZG_TILDE;
		}
		
		throw new RuntimeException("Undefined zawgyi: " + Integer.toHexString(unicodeChar));
	}
	
	public static SEMANTICS getSemantics(char unicodeChar) {
		if (unicodeChar < 0x1000) {
			if (unicodeChar==' ' || unicodeChar== ',' || unicodeChar== '.' )
				return SEMANTICS.SPACE_OR_PUNCT;
			else
				return SEMANTICS.NOT_MYANMAR;
		} else if (unicodeChar < 0x102B) {
			if (unicodeChar==0x1000 || unicodeChar==0x1001 || unicodeChar==0x1004 || unicodeChar==0x1005 ||
				unicodeChar==0x1007 || unicodeChar==0x100A || unicodeChar==0x100B || unicodeChar==0x100C || 
				unicodeChar==0x1010 || unicodeChar==0x1012 || unicodeChar==0x1014 || unicodeChar==0x1015 || 
				unicodeChar==0x1016 || unicodeChar==0x1017 || unicodeChar==0x1019 || unicodeChar==0x101A || 
				unicodeChar==0x1002 || unicodeChar==0x100F || unicodeChar==0x1010 || unicodeChar==0x101B || 
				unicodeChar==0x101C || unicodeChar==0x1018 || unicodeChar==0x101D || unicodeChar==0x101E || 
				unicodeChar==0x101F || unicodeChar==0x1025)
				return SEMANTICS.MY_PARTIAL;
			return SEMANTICS.MY_BASE;
		} else if (unicodeChar < 0x1040) {
			if (unicodeChar == 0x1031 || unicodeChar == 0x103B)
				return SEMANTICS.MY_LEADING;
			else if (unicodeChar == 0x1039)
				return SEMANTICS.MY_VIRAMA;
			else
				return SEMANTICS.MY_TRAILING;
		} else if (unicodeChar < 0x1056) {
			if (unicodeChar == 0x104A || unicodeChar == 0x104B)
				return SEMANTICS.MY_STOP;
			else
				return SEMANTICS.MY_BASE;			
		} else if (unicodeChar < 0x105B) {
			return SEMANTICS.MY_TRAILING;
		} else if (unicodeChar < 0x1200) {
			//Incomplete
			if (unicodeChar == 0x1060)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1061)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1062)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1064)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1065)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1066)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1067)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1068)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1069)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x106A)
				return SEMANTICS.MY_BASE;
			else if (unicodeChar == 0x106B)
				return SEMANTICS.MY_BASE;
			else if (unicodeChar == 0x106D)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1071)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1072)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1073)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1075)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1076)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1078)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x107B)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x107C)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x107D)
				return SEMANTICS.MY_TRAILING;
			else if (unicodeChar == 0x107E || unicodeChar == 0x107F || unicodeChar == 0x1080 || unicodeChar == 0x1081 || unicodeChar == 0x1082)
				return SEMANTICS.MY_LEADING;
			else if (unicodeChar == 0x1085)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x1086)
				return SEMANTICS.MY_BASE;
			else if (unicodeChar == 0x1087)
				return SEMANTICS.MY_TRAILING;
			else if (unicodeChar == 0x1088)
				return SEMANTICS.MY_TRAILING;
			else if (unicodeChar == 0x1089)
				return SEMANTICS.MY_TRAILING;
			else if (unicodeChar == 0x108A)
				return SEMANTICS.MY_TRAILING;
			else if (unicodeChar == 0x108B)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x108C)
				return SEMANTICS.MY_PAT_SINT;
			else if (unicodeChar == 0x108F)
				return SEMANTICS.MY_PARTIAL;
			else if (unicodeChar == 0x1090)
				return SEMANTICS.MY_BASE;
			else if (unicodeChar == 0x1091)
				return SEMANTICS.MY_BASE;
			else if (unicodeChar == 0x1092)
				return SEMANTICS.MY_BASE;
			else if (unicodeChar == 0x1094 || unicodeChar == 0x1095)
				return SEMANTICS.MY_TRAILING;
			else
				return SEMANTICS.MY_OTHER;
		} else {
			return SEMANTICS.NOT_MYANMAR;
		}
	}
		
}
