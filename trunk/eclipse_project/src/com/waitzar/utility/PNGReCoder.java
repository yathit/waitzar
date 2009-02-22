package com.waitzar.utility;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.zip.CRC32;

public class PNGReCoder {
	
	public class PNGChunk {
		public byte[] type;
		public byte[] data;
		public byte[] crc;
		
		public PNGChunk() {
			this.type = new byte[4];
			this.crc = new byte[4];
		}
		
		public byte[] getLength() {
			byte[] res = new byte[4];
			res[0] = (byte)((data.length>>24)&0xFF);
			res[1] = (byte)((data.length>>16)&0xFF);
			res[2] = (byte)((data.length>>8)&0xFF);
			res[3] = (byte)(data.length&0xFF);
			return res;
		}
	}
	
	
	public PNGReCoder(String[] args) {
		//Get our arguments
		String pngFilePath = "../win32_source/help_zg_main.font.png";
		if (args.length>0)
			pngFilePath = args[0];
		String widthsFilePath = "../win32_source/help_zg_main_widths.txt";
		if (args.length>1)
			widthsFilePath = args[1];
		
		//Do these files exist?
		File pngFile = new File(pngFilePath);
		File widthsFile = new File(widthsFilePath);
		if (!pngFile.exists()) {
			System.out.println("Error: File \"" + pngFilePath + "\" does not exist.");
			return;
		}
		if (!widthsFile.exists()) {
			System.out.println("Error: File \"" + widthsFilePath + "\" does not exist.");
			return;
		}
		
		//Read the PNG file into an array of chunks
		ArrayList<PNGChunk> chunks = new ArrayList<PNGChunk>();
		FileInputStream input = null;
		try {
			input = new FileInputStream(pngFile);
		} catch (FileNotFoundException ex) {
			System.out.println("Error: Couldn't find input file: " + pngFile.getAbsolutePath());
			return;
		}
		byte[] pngSig = new byte[] {(byte)0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
		byte[] fntChunk = new byte[] {0x66, 0x6f, 0x4e, 0x74};
		byte[] pulpFntMagicNum = new byte[] {0x70, 0x75, 0x6c, 0x70, 0x66, 0x6e, 0x74, 0x0b};
		try {
			//Read the png signature
			byte[] checkSig = new byte[pngSig.length];
			input.read(checkSig);
			for (int i=0; i<pngSig.length; i++) {
				if (pngSig[i] != checkSig[i]) {
					System.out.println("Error: PNG signature invalid!");
					return;
				}
			}
			
			for (;;) {
				//Anything left to read?
				byte[] rd_int = new byte[4];
				if (input.read(rd_int)!=rd_int.length)
					break;
				
				//Read a chunk
				PNGChunk nextCh = new PNGChunk();
				nextCh.data = new byte[((0xFF&rd_int[0])<<24) | ((0xFF&rd_int[1])<<16) | ((0xFF&rd_int[2])<<8) | ((0xFF&rd_int[3]))];
				input.read(nextCh.type);
				input.read(nextCh.data);
				input.read(nextCh.crc);
				
				//Double-check integrity
				CRC32 crc = new CRC32();
				crc.update(nextCh.type);
				crc.update(nextCh.data);
				long resCRC = crc.getValue();
				if ( ((resCRC>>24)&0xFF)!=((int)nextCh.crc[0]&0xFF) || ((resCRC>>16)&0xFF)!=((int)nextCh.crc[1]&0xFF) || ((resCRC>>8)&0xFF)!=((int)nextCh.crc[2]&0xFF) || (resCRC&0xFF)!=((int)nextCh.crc[3]&0xFF)) {
					System.out.println("Warning: Bad CRC for chunk " + new String(nextCh.type));
					System.out.println("  " + Long.toHexString(resCRC) + " to " + Integer.toHexString(((int)nextCh.crc[0])&0xFF) + "" + Integer.toHexString(((int)nextCh.crc[1])&0xFF) + "" + Integer.toHexString(((int)nextCh.crc[2])&0xFF) + "" + Integer.toHexString(((int)nextCh.crc[3])&0xFF));
				}
				
				//Add it?
				if (nextCh.type[0]==fntChunk[0] && nextCh.type[1]==fntChunk[1] && nextCh.type[2]==fntChunk[2] && nextCh.type[3]==fntChunk[3])
					System.out.println("Info: Skipping old font chunk");
				else
					chunks.add(nextCh);
			}
			
			//Done
			input.close();
		} catch (IOException ex) {
			System.out.println("Error reading file.");
			return;
		}
		
		//Process our widths file
		BufferedReader inputF;
		try {
			inputF = new BufferedReader(new FileReader(widthsFile));
		} catch (FileNotFoundException ex) {
			System.out.println("Error: Couldn't find file: " + widthsFile.getAbsolutePath());
			return;
		}
		ArrayList<Integer> widths = new ArrayList<Integer>();
		try {
			for (;;) {
				//Done?
				String line = inputF.readLine();
				if (line==null) 
					break;
				
				//Comment?
				line = line.trim();
				if (line.charAt(0)=='#')
					continue;
				
				//Number
				int value = 0;
				try {
					value = Integer.parseInt(line);
				} catch (NumberFormatException ex) {
					System.out.println("Bad number format: " + line);
					return;
				}
				widths.add(value);
			}
			inputF.close();
		} catch (IOException ex) {
			System.out.println("Can't read file: " + ex.toString());
			ex.printStackTrace();
			return;
		}
		if (widths.size()%61 != 0)
			System.out.println("Warning: Possibly missing some widths: " + widths.size());
		
		//Create our widths chunk, insert it before IEND
		//Pulp Core signature
		ArrayList<Byte> chunkData = new ArrayList<Byte>();
		for (byte b : pulpFntMagicNum)
			chunkData.add(b);
		
		//Start letter
		chunkData.add((byte)0);
		chunkData.add((byte)0);
		
		//Final letter
		int finalLetter = widths.size()-1;
		chunkData.add((byte)((finalLetter>>8)&0xFF));
		chunkData.add((byte)(finalLetter&0xFF));
		
		//Tracking
		chunkData.add((byte)0);
		chunkData.add((byte)0);
		
		//Has Bearing
		chunkData.add((byte)0);
		
		//Add our widths (including a special one at the end)
		int lastBearing = 0;
		for (int w : widths) {
			//Add this start position
			chunkData.add((byte)((lastBearing>>8)&0xFF));
			chunkData.add((byte)(lastBearing&0xFF));
			
			//Update to the next bearing
			lastBearing += w;
		}
		chunkData.add((byte)((lastBearing>>8)&0xFF));
		chunkData.add((byte)(lastBearing&0xFF));
		System.out.println("Image width should be: " + lastBearing + "  (plese check)");
		
		PNGChunk nextCh = new PNGChunk();
		nextCh.data = new byte[chunkData.size()];
		for (int i=0; i<chunkData.size(); i++)
			nextCh.data[i] = chunkData.get(i);
		nextCh.type = fntChunk;
		CRC32 crc = new CRC32();
		crc.update(nextCh.type);
		crc.update(nextCh.data);
		long nextCRC = crc.getValue();
		nextCh.crc[0] = (byte)((nextCRC>>24)&0xFF);
		nextCh.crc[1] = (byte)((nextCRC>>16)&0xFF);
		nextCh.crc[2] = (byte)((nextCRC>>8)&0xFF);
		nextCh.crc[3] = (byte)(nextCRC&0xFF);
		chunks.add(chunks.size()-2, nextCh);
		
		
		//Output the resulting PNG
		FileOutputStream output = null;
		try {
			output = new FileOutputStream(pngFile);
		} catch (Exception ex) {
			System.out.println("Error opening output file: " + ex.toString());
			ex.printStackTrace();
			return;
		}
		try {
			output.write(pngSig);
			for (PNGChunk ch : chunks) {
				output.write(ch.getLength());
				output.write(ch.type);
				output.write(ch.data);
				output.write(ch.crc);
			}
			
			output.close();
		} catch (IOException ex) {
			System.out.println("Error writing to output file.");
			return;
		}
		
		
		System.out.println("Done");

	}
	

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		new PNGReCoder(args);
	}

}
