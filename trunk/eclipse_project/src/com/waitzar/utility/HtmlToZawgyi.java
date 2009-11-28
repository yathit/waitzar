package com.waitzar.utility;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JTextArea;
import javax.swing.JTextField;


/**
 * Document later: helper class for email conversion
 * @author Seth N. Hetu
 *
 */
public class HtmlToZawgyi extends JFrame {
	private JButton jb;
	private JTextArea ta;
	
	public HtmlToZawgyi() {
		super("Html 2 Zawgyi");
		
		//Setup
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setSize(400, 300);
		this.setLocation(200, 150);

		//Add children
		this.setLayout(new BorderLayout());
		this.addControls(this.getContentPane());
		
        //Show
		//this.pack();
		this.setVisible(true);
	}
	
	private void addControls(Container cp) {
		//Temp:
		//JTextField text2 = new JTextField("", 15);
		//text2.setFont(new Font("Padauk", Font.BOLD, 12));
		//cp.add(BorderLayout.NORTH, text2);

		
		ta = new JTextArea();
		ta.setFont(new Font("Zawgyi-One", Font.PLAIN, 12));
		cp.add(BorderLayout.CENTER, ta);
		
		jb = new JButton("Convert");
		jb.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				//Regex's are failing me...
				StringBuilder sb = new StringBuilder();
				char[] txt = ta.getText().toCharArray();
				for (int i=0; i<txt.length; i++) {
					boolean pass = true;
					if (txt[i]=='%') {
						//Easiest just to try it.
						try {
							if (txt[i+1]=='u' && txt[i+2]=='1' && txt[i+3]=='0') {
								String num = "10" + txt[i+4] + "" + txt[i+5];
								int numI = Integer.parseInt(num, 0x10);
								sb.append((char)numI);
								i += 5;
								pass = false;
							}
						} catch (NumberFormatException nex) {
						} catch (ArrayIndexOutOfBoundsException aex) {}
					}
					
					if (pass) {
						sb.append(txt[i]);
					}
				}
				
				//Done
				ta.setText(sb.toString());
			}
		});
		cp.add(BorderLayout.SOUTH, jb);
	}


	
	
	public static void main(String[] args) {
		new HtmlToZawgyi();
	}

}
