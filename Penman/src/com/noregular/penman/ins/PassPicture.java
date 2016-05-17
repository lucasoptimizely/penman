package com.noregular.penman.ins;

import org.opencv.core.Mat;
import org.opencv.features2d.KeyPoint;

import android.graphics.Bitmap;

/**
 * Singleton to pass a picture to the next activity.
 * 
 * @author Lucas Swartsenburg (6174388)
 *
 */
public class PassPicture {
	private static PassPicture _instance = null;
	private Mat image;
	private String SVM = "";
	private byte[] temp;
	private KeyPoint[] k;
	private int width, height;
	private Mat keypointsMat;
	private Bitmap bitmap;
	private String location;

	/**
	 * Std constructor
	 */
	private PassPicture() {
		
	}

	/**
	 * Returns a instance for passing a picture.
	 */
	private synchronized static void createInstance() {
		if (_instance == null)
			_instance = new PassPicture();
	}

	/**
	 * Returns the singleton.
	 * @return
	 */
	public static PassPicture getInstance() {
		if (_instance == null)
			createInstance();
		return _instance;
	}


	/**
	 * @return the picture
	 */
	public Mat getPicture() {
		return image;
	}

	/**
	 * 
	 * @param picture
	 */
	public void setPicture(Mat picture) {
		this.image = picture;
	}

	public KeyPoint[] getK() {
		return k;
	}

	public void setK(KeyPoint[] k) {
		this.k = k;
	}

	public byte[] getTemp() {
		return temp;
	}

	public void setTemp(byte[] temp) {
		this.temp = temp;
	}

	public Mat getKeypointsMat() {
		return keypointsMat;
	}

	public void setKeypointsMat(Mat keypointsMat) {
		this.keypointsMat = keypointsMat;
	}

	public int getWidth() {
		return width;
	}

	public void setWidth(int width) {
		this.width = width;
	}

	public int getHeight() {
		return height;
	}

	public void setHeight(int height) {
		this.height = height;
	}

	public Bitmap getBitmap() {
		return bitmap;
	}

	public void setBitmap(Bitmap bitmap) {
		this.bitmap = bitmap;
	}

	public String getLocation() {
		return location;
	}

	public void setLocation(String location) {
		this.location = location;
	}

	public String getSVM() {
		return SVM;
	}

	public void setSVM(String sVM) {
		SVM = sVM;
	}
	
	public void clear(){
		image = null;
		SVM = null;
		temp = null;
		k = null;
		keypointsMat = null;
		bitmap = null;
		location = null;	
		System.gc();
	}
	

}
