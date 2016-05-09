/**
 * 
 */
package org.waxbee;

import java.awt.Color;
import java.awt.Container;
import java.awt.Desktop;
import java.awt.FileDialog;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Panel;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.URI;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.Vector;

import javax.imageio.ImageIO;
import javax.swing.DefaultComboBoxModel;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.JSeparator;
import javax.swing.JToolBar;

import net.miginfocom.swing.MigLayout;

import org.waxbee.teensy.FirmwareImage;
import org.waxbee.teensy.extdata.BuildVariant;
import org.waxbee.teensy.extdata.ConfigFileParser;
import org.waxbee.teensy.extdata.ConfigFileWriter;
import org.waxbee.teensy.extdata.ConfigItem;
import org.waxbee.teensy.extdata.ConfigTemplate;
import org.waxbee.teensy.extdata.ExtDataConstant;
import org.waxbee.teensy.extdata.WaxBeeConfigEncoder;
import org.waxbee.teensy.extdata.WaxbeeConfig;

/**
 * @author Bernard
 * 
 */
@SuppressWarnings("serial")
public class MainApplication extends JFrame
{
	private static final String VERSION = "0.16"; // Keep version in sync inside strings.h !!

	private static final String TEMPLATE_EXTENSION_PATTERN = "*.tmpl.txt";

	private static final String WAXBEE_CONFIG_TXT = "WaxbeeConfig.txt";

	public static MainApplication theMainApp;

	WaxbeeConfig itsWaxbeeConfig;
	List<FirmwareVariant> itsFirmwareVariants;
	private FirmwareVariant itsSelectedFirmware;
	
	private JComboBox itsMonitorsCombo;

	private JLabel itsConfigNameLabel;
	private JLabel itsConfigDescLabel;
	private JLabel itsConfigRequiresLabel;

	private JComboBox itsFirmwareVariantCombo;
	private JLabel itsConfigSupportsLabel;

	private JLabel itsCoordinatesMappingSummaryLabel;
	
	public MainApplication()
	{
		theMainApp = this;


		createMenu();

		Container cp = getContentPane();

		cp.setBackground(Color.WHITE);
		cp.setLayout(new MigLayout("wrap 1, insets 0", // layout constrains: 1
														// component per row, no
														// insets
				"[10px::,grow,fill]", // column constraints: col grows when frame is resized
				"[align top,grow][30px::]"));  // rows constraints: [30px::] is for the bottom status bar
		
		Container p = new Panel(new MigLayout("wrap 1","[10px::]",""));

		cp.add(p);

		addSeparator(p, "Configuration");

		Container p2 = new Panel(new MigLayout("wrap 2","[][10px::,grow]",""));
		p.add(p2, "wmin 1,span,growx");
		p2.add(new JLabel("Name :"),"gap indent, aligny top");
		p2.add(itsConfigNameLabel = new JLabel(""), "wmin 1,wrap");
		
		p2.add(new JLabel("Description :"),"gap indent, aligny top");
		p2.add(itsConfigDescLabel = new JLabel(""), "wmin 1,growx,wrap");

		p2.add(new JLabel("Requires :"),"gap indent, aligny top");
		p2.add(itsConfigRequiresLabel = new JLabel(""), "wmin 1,growx,wrap");
		
		addSeparator(p, "Firmware Selection");
		
		JCheckBox check = new JCheckBox("Auto select firmware based on requirements",true);
		p.add(check, "gap indent");

		itsFirmwareVariantCombo = new JComboBox();
		itsFirmwareVariantCombo.setEnabled(false);
		//itsFirmwareVariantCombo.setSelectedIndex(0);
		p.add(itsFirmwareVariantCombo, "gap indent");
		
		check.setOpaque(false);
        check.addItemListener(new ItemListener() {
			@Override public void itemStateChanged(ItemEvent e)
			{
                itsFirmwareVariantCombo.setEnabled(e.getStateChange() != ItemEvent.SELECTED);
                if(e.getStateChange() == ItemEvent.SELECTED) {
	                try {
	                	autoPickFirmwareIfRequired();
	                } catch(Exception ex) {}
	            	displayConfig();	
                }
			}});

        itsFirmwareVariantCombo.addItemListener(new ItemListener() {
			@Override public void itemStateChanged(ItemEvent ev)
			{
				if(ev.getStateChange() == ItemEvent.SELECTED) {
					
					FirmwareVariant v = (FirmwareVariant)ev.getItem();
					if(v != null && v != itsSelectedFirmware) {
						itsSelectedFirmware = v;
						displayConfig();
					}
				}
			}});
		p.add(itsConfigSupportsLabel = new JLabel("Supports :"), "gap indent, growx, width 10px:400px:");
        
		/*addSeparator(p, "Coordinates Mapping");*/

		/*p.add*(*/itsCoordinatesMappingSummaryLabel = new JLabel("");/*, 
				"gap indent, growx, width 10px:300px:";);*/
		
		/*p.add(new JLabel("<html>Tip: For <u>special coordinates mapping</u>** to work correctly, " +
				"the settings of the native (Wacom) USB driver must map the whole tablet area " +
				"to the entire <b>target monitor</b> and without any special mapping feature " +
				"like keeping the proportions.<p> <i>**Tip not applicable to 'full mapping' mode since it implies no special coordinates mapping"), 
				"gap para, growx, width 10px:300px:");

		p.add(new JLabel("Target Monitor:"), "split 2, gap indent");
		itsMonitorsCombo = new JComboBox(new Vector<Monitor>(fetchMonitors()));
		itsMonitorsCombo.setSelectedIndex(0);
		p.add(itsMonitorsCombo, "wmin 40,grow 0,wrap");
*/
		setIconImage(getImage("appicon.png"));
		setTitle("Waxbee Config " + VERSION);

		try
		{
			itsWaxbeeConfig = ConfigFileParser.loadFile(WAXBEE_CONFIG_TXT);
		}
		catch(Exception e)
		{
			itsWaxbeeConfig = ConfigTemplate.create();
		}

		try {
			autoPickFirmwareIfRequired();
		}
		catch(Exception ex) {}
		
		displayConfig();
		pack();
		setLocationRelativeTo(null);
		
        addWindowListener(new WindowAdapter() 
        {
            public void windowClosing(WindowEvent e) 
            {
                System.exit(0);
            }
        });
	}

	private void createMenu()
	{
		JMenuBar menuBar = new JMenuBar();
		JMenu menu;
		JMenuItem mi;
		JMenu submenu;
		
		menuBar.add(menu = new JMenu("File"));
		menu.setMnemonic(KeyEvent.VK_F);
		menu.add(mi = new JMenuItem("New from Template...", KeyEvent.VK_N));
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					String filename = fileDialog(false, TEMPLATE_EXTENSION_PATTERN, computeTemplateDir());
					
					if(filename != null)
					{
						itsWaxbeeConfig = ConfigFileParser.loadFile(filename);

						autoPickFirmwareIfRequired();
						saveConfig();
						displayConfig();
						pack();
					}
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}

		});
		menu.addSeparator();
		menu.add(mi = new JMenuItem("Save as Template...", KeyEvent.VK_S));
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					String filepath = fileDialog(true, TEMPLATE_EXTENSION_PATTERN, computeTemplateDir());
					
					if(filepath == null)
						return;

					new ConfigFileWriter(itsWaxbeeConfig).save(new File(filepath));

					JOptionPane.showMessageDialog(MainApplication.this, "Template file written succesfully.");
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
		menu.addSeparator();
		menu.add(mi = new JMenuItem("Raw Config Editing..."));
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					ExtDataEditor editor;
					editor = new ExtDataEditor(MainApplication.this, itsWaxbeeConfig);
					editor.setVisible(true);
					
					autoPickFirmwareIfRequired();
					saveConfig();
					displayConfig();
					pack();
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
		menu.addSeparator();
		menu.add(mi = new JMenuItem("Exit"));
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				MainApplication.this.dispose();
			}
		});

		menuBar.add(menu = new JMenu("Mapping"));
		menu.add(submenu = new JMenu("Orientation"));
		submenu.setEnabled(false);
		submenu.add(mi = new JRadioButtonMenuItem("Normal", true));
		submenu.add(mi = new JRadioButtonMenuItem("Rotated Right"));
		submenu.add(mi = new JRadioButtonMenuItem("Upside Down"));
		submenu.add(mi = new JRadioButtonMenuItem("Rotated Left"));
		menu.add(mi = new JMenuItem("Setup active area..."));
		mi.setEnabled(false);
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					CoordinatesEditor editor;
					editor = new CoordinatesEditor(MainApplication.this, itsWaxbeeConfig, 
							(Monitor)itsMonitorsCombo.getSelectedItem());
					editor.setVisible(true);
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
/*
		menu.add(mi = new JMenuItem("Test Swing Painting..."));
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					FastPaintTest editor;
					editor = new FastPaintTest(MainApplication.this, itsWaxbeeConfig, 
							(Monitor)itsMonitorsCombo.getSelectedItem());
					editor.setVisible(true);
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
*/		
		
		menu.add(mi = new JMenuItem("Map unreachable edges..."));
		mi.setEnabled(false);

		menuBar.add(menu = new JMenu("Firmware"));
		menu.add(mi = new JMenuItem("Program Device..."));
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					ProgramDialog dlg = new ProgramDialog(MainApplication.this);
					dlg.program(getSelectedFirmware().getFirmwareName(), itsWaxbeeConfig);
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});

		menu.add(submenu = new JMenu("Program Device with Test firmware"));
		submenu.add(mi = new JMenuItem("'Slow' Blinky..."));
		mi.setEnabled(false);
		submenu.add(mi = new JMenuItem("'Fast' Blinky..."));
		mi.setEnabled(false);
		submenu.add(mi = new JMenuItem("Other firmware..."));
		mi.setEnabled(false);
		menu.add(mi = new JMenuItem("Generate firmware '.hex' file..."));
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					String filepath = fileDialog(true, "*.hex", null);
					
					if(filepath == null)
						return;
					
					StringBuilder report = new StringBuilder();
					
					FirmwareImage outputImage = encodeFirmware(getSelectedFirmware().getFirmwareName(), itsWaxbeeConfig, report);

					FileWriter out = new FileWriter(filepath);
					outputImage.generateIntelHex(out);
					out.close();

					JOptionPane.showMessageDialog(MainApplication.this, "Hex file written successfully.\n\nDetailed Report :\n" + report.toString());
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
		menu.add(submenu = new JMenu("Test Tools"));
		submenu.add(mi = new JMenuItem("Raw HID sniffer..."));
		mi.setEnabled(false);
		submenu.add(mi = new JMenuItem("Debug Console (HID Listen)..."));
		mi.setEnabled(false);
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					HidListenWindow dlg = new HidListenWindow(MainApplication.this);

					dlg.setVisible(true);
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
		
		menuBar.add(menu = new JMenu("Help"));
		menu.add(mi = new JMenuItem("WaxBee Project Site"));
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					Desktop.getDesktop().browse(new URI("http://code.google.com/p/waxbee/"));
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
		menu.add(mi = new JMenuItem("forum.bongofish.co.uk"));
		mi.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					Desktop.getDesktop().browse(new URI("http://forum.bongofish.co.uk"));
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
		
		menu.addSeparator();
		menu.add(mi = new JMenuItem("About WaxBee"));
		mi.setEnabled(false);

		setJMenuBar(menuBar);
	}

	protected String fileDialog(boolean isForSaving, String filepattern, File directory)
	{
		FileDialog dlg = new FileDialog(MainApplication.this);

		if(isForSaving)
			dlg.setMode(FileDialog.SAVE);
		else
			dlg.setMode(FileDialog.LOAD);
		
		dlg.setFile(filepattern);
		if(directory != null)
			dlg.setDirectory(directory.getAbsolutePath());
		dlg.setVisible(true);

		String filename = dlg.getFile();
		
		if(filename == null)
			return null;

		String filepath = (new File(dlg.getDirectory()).getAbsolutePath()) + File.separator + filename;
		
		return filepath;
	}

	protected File computeTemplateDir()
	{
		return new File("config_templates");
	}

	static final Color LABEL_COLOR = new Color(0, 70, 213);

	private void addSeparator(Container panel, String text)
	{
		JLabel l = new JLabel(text);
		l.setForeground(LABEL_COLOR);

		panel.add(l, "gapbottom 1, span, split 2, aligny center");
		panel.add(new JSeparator(), "gapleft rel, growx");
	}

	protected void autoPickFirmwareIfRequired() throws Exception
	{
		String[] requires = itsWaxbeeConfig.getRequires();

		// *** only if autoselect checked...
		
		List<FirmwareVariant> variants = fetchFirmwareVariants();
		
		nextvariant: 
			for(FirmwareVariant variant : variants) {
				Set<String> supports = variant.getSupports();
				for(int i=0;i<requires.length;i++) {
					if(!supports.contains(requires[i]))
						continue nextvariant;
				}
				// got a matching variant
				itsSelectedFirmware = variant;
				return;
			}
		throw new Exception("Could not find a firmware image with all the Requirements (that's most likely a bug)");
	}

	private void displayConfig()
	{
		try
		{
			itsConfigNameLabel.setText("<html>"+optionalStr(itsWaxbeeConfig.getName()));
			itsConfigDescLabel.setText("<html>"+optionalStr(itsWaxbeeConfig.getDescription()));
			itsConfigRequiresLabel.setText("<html>"+listStringArray(itsWaxbeeConfig.getRequires()));
			
			itsFirmwareVariantCombo.setModel(new DefaultComboBoxModel(new Vector<FirmwareVariant>(fetchFirmwareVariants())));
			itsFirmwareVariantCombo.setSelectedItem(getSelectedFirmware());
			itsConfigSupportsLabel.setText("<html>Supports : "+listStringArray(getSelectedFirmware().itsBuild.getSupports()));
			
			StringBuilder mapping = new StringBuilder("<html>Mode : ");
	
			if(	itsWaxbeeConfig.getConfigItemInt(ExtDataConstant.EXTDATA_USB_X_MAX) == itsWaxbeeConfig.getUSBTabletXMax()  && 
				itsWaxbeeConfig.getConfigItemInt(ExtDataConstant.EXTDATA_USB_X_MIN) == 0 &&
				itsWaxbeeConfig.getConfigItemInt(ExtDataConstant.EXTDATA_USB_Y_MAX) == itsWaxbeeConfig.getUSBTabletYMax()  && 
				itsWaxbeeConfig.getConfigItemInt(ExtDataConstant.EXTDATA_USB_Y_MIN) == 0 &&
				itsWaxbeeConfig.getConfigItemInt(ExtDataConstant.EXTDATA_SLAVE_X_MAX) == itsWaxbeeConfig.getSlaveTabletXMax()  && 
				itsWaxbeeConfig.getConfigItemInt(ExtDataConstant.EXTDATA_SLAVE_X_MIN) == 0 &&
				itsWaxbeeConfig.getConfigItemInt(ExtDataConstant.EXTDATA_SLAVE_Y_MAX) == itsWaxbeeConfig.getSlaveTabletYMax()  && 
				itsWaxbeeConfig.getConfigItemInt(ExtDataConstant.EXTDATA_SLAVE_Y_MIN) == 0 &&
				itsWaxbeeConfig.getConfigItemInt(ExtDataConstant.EXTDATA_SLAVE_ORIENTATION) == ExtDataConstant.EXTDATA_SLAVE_ORIENTATION_NORMAL)
			{
				mapping.append("<b>Full Mapping</b> <i>(Total active area mapped to whole USB tablet with normal orientation)");
			}
			else
			{
				mapping.append("<b>Special Mapping</b>");
				mapping.append("; Orientation : ");
				mapping.append(((ConfigItem.EnumItem)itsWaxbeeConfig.getConfigItem(ExtDataConstant.EXTDATA_SLAVE_ORIENTATION)).currentEnum().getName());
			}
	
			itsCoordinatesMappingSummaryLabel.setText(mapping.toString());
		}
		catch (Exception ex)
		{
			JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
		}

	}
	
	private String listStringArray(String[] items) 
	{
		if(items == null)
			return "--";
		
		StringBuilder sbStr = new StringBuilder();
	    for (int i = 0, il = items.length; i < il; i++) {
	        if (i > 0)
	            sbStr.append(", ");
	        sbStr.append(items[i]);
	    }
	    return sbStr.toString();
	}
	
	private String optionalStr(String str)
	{
		if(str == null)
			return "n/a";
		
		return str;
		
	}
	
	public JToolBar createToolbar()
	{
		JToolBar bar = new JToolBar();
		bar.setFloatable(false);

		JButton b = new JButton("Settings", getIcon("appicon.png"));
		// b.setToolTipText(Messages.getString("select_problems"));
		b.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					ExtDataEditor editor;
					editor = new ExtDataEditor(MainApplication.this, itsWaxbeeConfig);
					editor.setVisible(true);
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
		bar.add(b);

		b = new JButton("Coordinates", getIcon("appicon.png"));
		// b.setToolTipText(Messages.getString("select_problems"));
		b.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				try
				{
					CoordinatesEditor editor;
					editor = new CoordinatesEditor(MainApplication.this, itsWaxbeeConfig, 
							(Monitor)itsMonitorsCombo.getSelectedItem());
					editor.setVisible(true);
				}
				catch (Exception ex)
				{
					JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
				}
			}
		});
		bar.add(b);

		b = new JButton("Program", getIcon("appicon.png"));
		// b.setToolTipText(Messages.getString("select_problems"));
		// b.addActionListener(new ActionListener() {
		// public void actionPerformed(ActionEvent e) {
		// }
		// });
		bar.add(b);

		return bar;
	}

	public ImageIcon getIcon(String imageName)
	{
		String imagePath = "/org/waxbee/" + imageName;
		URL url = getClass().getResource(imagePath);
		return new ImageIcon(url);
	}

	public BufferedImage getImage(String imageName)
	{
		try
		{
			String imagePath = "/org/waxbee/" + imageName;
			URL url = getClass().getResource(imagePath);
			return ImageIO.read(url);
		}
		catch(IOException ex)
		{
			JOptionPane.showMessageDialog(MainApplication.this, ex.getMessage());
			return null;
		}
	}

	private void saveConfig() throws Exception
	{
		new ConfigFileWriter(itsWaxbeeConfig).save(new File(WAXBEE_CONFIG_TXT));
	}
	
	class FirmwareVariant {
		@Override
		public String toString()
		{
			double kilos = ((double)itsFirmwareSize) / 1024.0;
			return itsBuild.getName() + String.format(Locale.ENGLISH, "   (%.2fK)", kilos);
		}

		BuildVariant itsBuild;
		
		/** .hex suffix included */
		String itsFirmwareName;
		
		int	itsFirmwareSize;
		HashSet<String> itsSupports;
		
		public FirmwareVariant(BuildVariant build, String name, int size)
		{
			itsBuild = build;
			itsFirmwareName = name;
			itsFirmwareSize = size;
		}

		public String getFirmwareName() { return itsFirmwareName; }		
		public int getFirmwareSize() { return itsFirmwareSize; }
		public Set<String> getSupports() {
			if(itsSupports == null) {
				String[] array = itsBuild.getSupports();
				HashSet<String> supports = new HashSet<String>(array.length);
				for(int i=0;i<array.length;i++) {
					supports.add(array[i]);
				}
				itsSupports = supports;
			}
			
			return itsSupports;
		}
	}
	
	private List<FirmwareVariant> fetchFirmwareVariants() throws Exception
	{
		if(itsFirmwareVariants == null)
		{
			List<BuildVariant> builds = ConfigTemplate.createBuildVariantList();
	
			ArrayList<FirmwareVariant> variants = new ArrayList<FirmwareVariant>(builds.size());
	
			for(BuildVariant build : builds) {
				String firmwareName = build.getName() + ".hex";
				variants.add(new FirmwareVariant(build, firmwareName, fetchFirmwareSize(firmwareName)));
			}
			
			Collections.sort(variants, new Comparator<FirmwareVariant>() {

				@Override
				public int compare(FirmwareVariant a, FirmwareVariant b)
				{
					return a.itsFirmwareSize - b.itsFirmwareSize;
				}				
			});
			
			itsFirmwareVariants = variants;
		}
		
		return itsFirmwareVariants;
	}

	private int fetchFirmwareSize(String name) throws Exception
	{
		return loadFirmware(name).getSpanSize();
	}

	/**
	 * 
	 * @return list of monitors. The first entry corresponds to the primary monitor
	 */
	private List<Monitor> fetchMonitors()
	{
		ArrayList<Monitor> monitors = new ArrayList<Monitor>();
		
		GraphicsEnvironment e = GraphicsEnvironment.getLocalGraphicsEnvironment();

		GraphicsDevice primary = e.getDefaultScreenDevice();
		for (GraphicsConfiguration config : primary.getConfigurations())
		{
			Rectangle gcBounds = config.getBounds();
			monitors.add(new Monitor(primary.getIDstring(),gcBounds));
			// just pick the first one
			break;
		}
		
		GraphicsDevice[] devices = e.getScreenDevices();

		for (GraphicsDevice device : devices)
		{
			if(!device.getIDstring().equals(primary.getIDstring()))
			{
				for (GraphicsConfiguration config : device.getConfigurations())
				{
					Rectangle gcBounds = config.getBounds();
					monitors.add(new Monitor(device.getIDstring(),gcBounds));
					// just pick the first one
					break;
				}
			}
		}
		
		return monitors;
	}

	public static FirmwareImage encodeFirmware(String firmwarename, WaxbeeConfig waxbeeConfig, Appendable report) throws Exception
	{
		FirmwareImage firmwareImage = loadFirmware(firmwarename);

		WaxBeeConfigEncoder waxbeeConfigEncoder = new WaxBeeConfigEncoder();
		
		FirmwareImage outputImage = waxbeeConfigEncoder.encodeConfig(firmwareImage, waxbeeConfig, report);
		return outputImage;
	}

	private static FirmwareImage loadFirmware(String name) throws UnsupportedEncodingException,
			Exception, IOException
	{
		InputStream is = MainApplication.class.getClassLoader().getResourceAsStream(name);
		
		InputStreamReader file = new InputStreamReader(is, "UTF-8");
		FirmwareImage firmwareImage = new FirmwareImage();
		firmwareImage.loadIntelHex(file);
		file.close();
		return firmwareImage;
	}

	public FirmwareVariant getSelectedFirmware() { 
		if(itsSelectedFirmware == null) 
			if(itsFirmwareVariants != null)
				itsSelectedFirmware = itsFirmwareVariants.get(0);

		return itsSelectedFirmware;
	}
}
