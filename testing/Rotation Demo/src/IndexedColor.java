public class IndexedColor {
	//No-argument constructor; required for gson
	private IndexedColor() {}
	
	//Typical constructor
	public IndexedColor(String id, String rgb) {
		this(); //Added to avoid a build warning.
		this.id = id;
		this.rgb = rgb;
	}
	
	private String id;
	
	private String rgb;
	
	public String getId() { return id; }
	public String getRGB() { return rgb; }
}
