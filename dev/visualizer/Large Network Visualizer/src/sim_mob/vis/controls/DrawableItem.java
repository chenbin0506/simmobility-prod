package sim_mob.vis.controls;

import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;

/**
 * 
 * \author Seth N. Hetu
 */
public interface DrawableItem {
	///Helper z-orders
	public static final int Z_ORDER_NODE          =  10;
	public static final int Z_ORDER_LINK          =  20;
	public static final int Z_ORDER_SEGMENT       =  30;
	public static final int Z_ORDER_CUTLINE       =  40;
	public static final int Z_ORDER_SEGMENT_NAMES =  45;
	public static final int Z_ORDER_LANEMARKING   =  50;
	public static final int Z_ORDER_CROSSING      =  60;
	public static final int Z_ORDER_TSC           =  70;
	public static final int Z_ORDER_TSL           =  80;
	public static final int Z_ORDER_ANNOTATION    =  90;
	public static final int Z_ORDER_BusStop       = 100;
	public static final int Z_ORDER_TRAFFIC_SIGNAL= 110;
	public static final int Z_ORDER_SD_VERTEX     = 111;
	public static final int Z_ORDER_SD_EDGE       = 112;
	
	//We don't really care about the order of drivers/pedestrians
	public static final int Z_ORDER_DRIVER        = 400;
	public static final int Z_ORDER_PEDESTIRAN    = 400;
	public static final int Z_ORDER_TRACKINGITEM  = 410;
	public static final int Z_ORDER_TRAFFIC_SIGNAL_UPDATE = 420;
	public static final int Z_ORDER_PASSENGER = 430;
	
	///Draw this item.
	public void draw(Graphics2D g, DrawParams params);
	
	
	///Returns the bounds (in real, unscaled x,y coordinates) of this item.
	///  These bounds are not expected to change; if they do change, then the 
	///  code that relies on this items' bounds must be checked manually.
	public Rectangle2D getBounds();
	
	//Returns the Z-order of this component. Lower items are drawn first.
	public int getZOrder();
}
