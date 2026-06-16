<h1 align=center>
  📡 Micro Radar
</h1>
<h6 align=center>
  a tiny open-source flight radar for your desk
</h6>
<p align=center>
  <img src="https://github.com/user-attachments/assets/52e1c6fd-17ba-4838-9518-a1c8790af4a8" alt="drawing" width="400"/>
</p>
<p align=center>
  <a href="#prerequisites">PREREQUISITES</a> - <a href="#assembly">ASSEMBLY</a> - <a href="#usage">USAGE</a>
</p>

## Prerequisites

At the core of this project is the ESP32-C3 module with an integrated 240x240 IPS screen. No soldering required. The module does all the heavy lifting.

I used dark grey PLA filament for the print, but any colour will work just fine. For the lens (optional but recommended), you'll need clear-drying epoxy to secure it (not super glue, which will fog up the lens. Ask me how I know.)

### Tools you'll need

- Small screwdriver (for M2 screws)
- Soldering iron (for setting the threaded inserts)

Set up a clean, organised workspace before you start. You'll be handling small components and epoxy, so a dedicated area helps. Keep a damp cloth nearby for cleaning if needed, and ensure your soldering iron has adequate ventilation.

### Shopping List

Everything you need is below. I've linked products I used and recommend for ease of build, though alternatives exist on Amazon and elsewhere. If you deviate from this hardware, you may need to modify the enclosure and/or code.

- [ ] [1.28" Round GC9A01 240x240 IPS Display Module with ESP32-C3 (no-touch)](https://www.aliexpress.com/item/1005008482665220.html)
- [ ] [USB-C Ribbon Extension Cable (5cm, CMUP-CFPCB-BK)](https://www.aliexpress.com/item/1005005371248824.html)
- [ ] [M2 Heat-set Threaded Inserts (+ soldering iron)](https://www.aliexpress.com/item/1005008493831823.html)
- [ ] [32.5mm Round Mineral Glass Lens (optional, recommended)](https://www.aliexpress.com/item/1005004783554496.html)
- [ ] [Gorilla Epoxy (necessary for fitting lens, useful anyway)](https://www.amazon.co.uk/Gorilla-Glue-25ml-Epoxy/dp/B009NQQJFC)

### Accounts / API

This project uses OpenSky's API for retrieving flight data.

I highly recommend making an account, as it's free, and allows the radar to make many more requests per day (400 -> 4000), which makes the live view much more accurate. However, it isn't necessary if you prefer.

You can sign up [here](https://opensky-network.org), or search "OpenSky".

Further info on what to do with the account is in the usage section.

## Assembly

Once you've got all the parts, assembly typically takes 1-2 hours (excluding print time).

**I strongly recommend reading the [Usage](#usage) section before you start assembly.** It'll help with troubleshooting if anything goes wrong. You might want to test the firmware and your hardware before closing everything up.

### Step 1: 3D Print

<img width="400" alt="FFCBBECA-6165-4138-8C84-16AB375511A2_1_105_c" src="https://github.com/user-attachments/assets/21c0753c-7d7c-425c-bdf6-0df037a8fdaa" />

Print all four STLs from `./hardware/stl/`:

- Main enclosure
- Front plate
- Bezel
- 2 spacers

### Step 2: Heat-set Threaded Inserts

**You'll need:** Soldering iron, M2 threaded inserts

Start with the front plate: insert 2mm M2 threaded inserts into the larger holes using the soldering iron.

<img width="400" alt="IMG_7882" src="https://github.com/user-attachments/assets/defcfb2c-cdff-4bf1-84b9-7fceeefb0caf" />

Next, the two spacers. These might warp slightly, that's fine. Insert 6mm M2 inserts into each.

<img width="400" alt="IMG_7887" src="https://github.com/user-attachments/assets/73b95049-5f12-4e2b-983a-5242c05f9106" />

Finally, the main enclosure. Insert 5mm M2 inserts.

<img width="400" alt="IMG_7891" src="https://github.com/user-attachments/assets/e36f3eec-31b5-468e-8451-9c428eaf9c21" />

Et voilà.

<img width="400" alt="IMG_7896" src="https://github.com/user-attachments/assets/97337223-223c-4531-90e1-f511adfb3d66" />

### Step 3 (Optional): Fitting the Lens

<img width="400" alt="IMG_7902" src="https://github.com/user-attachments/assets/e555f787-ca87-4558-b1eb-107f9071f96e" />

**You'll need:** Clear-drying epoxy, small applicator (match or cocktail stick works)

This is the fiddliest bit. Keep it neat and you'll avoid frustration:

- Apply epoxy to the front plate, not the lens
- Lower the front plate onto the lens (easier to manage excess epoxy)
- Have a cleaner ready for the edges (I used nail polish remover, your mileage may vary)
- Less is more with epoxy
- Work on a surface that won't bond to epoxy

<img width="400" alt="IMG_7911" src="https://github.com/user-attachments/assets/aa497389-efd5-45c3-84dc-c997232889ac" />

Let the epoxy cure according to its label before moving on.

### Step 4: Bezel

**You'll need:** 2x5mm M2 screws, 2x10mm M2 screws

Secure the bezel to the front plate using 2x5mm M2 screws through the threaded inserts you added earlier.

<img width="400" alt="IMG_7914" src="https://github.com/user-attachments/assets/37a3502a-83e1-4552-a399-9a914e0ec973" />

Screw 2x10mm M2 screws through the remaining two holes. They should protrude from the back.

<img width="400" alt="IMG_7915" src="https://github.com/user-attachments/assets/9ccfe5f2-347d-4563-a2b1-eb5e65e1d83f" />

Peel the protective film off the screen and position it over the lens. The screws you just inserted will guide the display into place.

Before clamping it down with the spacers, **make sure the antenna is attached to the module**. Press it down firmly onto the flat surface until it clicks. Orientation doesn't matter, but you do need this connection otherwise you won't get any WiFi signal.

<img width="400" alt="IMG_7917" src="https://github.com/user-attachments/assets/ee53aac0-d119-4941-a814-f7ef23ffe7a0" />

<img width="400" alt="IMG_7920" src="https://github.com/user-attachments/assets/0d4d7d86-9787-4972-aa55-8ae43c9a078b" />

Now screw the spacers into those protruding screws to clamp the module in place. I recommend keeping the board plugged in at this point to help with alignment. Don't use much force, too much pressure will stress the screen.

Once you're happy with the alignment, you can add a small amount of epoxy around the module to lock it in place permanently (optional, but recommended if you're happy with the build).

### Step 5: Final Assembly

**You'll need:** USB-C ribbon extension cable, 4x7mm M2 screws, optional rubber feet

Attach the USB-C ribbon cable to the case with the provided nuts and bolts.

<img width="400" alt="IMG_7921" src="https://github.com/user-attachments/assets/f40a7943-c880-4718-9e69-c87a4f5d33aa" />

<img width="400" alt="IMG_7923" src="https://github.com/user-attachments/assets/2daccb36-421f-4a3e-812a-51dae4444d4e" />

If you like, remove the supports from the bottom and insert rubber feet.

<img width="400" alt="IMG_7924" src="https://github.com/user-attachments/assets/fdeb69f2-ec0d-441e-95ca-abd7523f7c61" />

Plug in the board, then attach the front plate using 4x7mm M2 screws.

<img width="400" alt="IMG_7925" src="https://github.com/user-attachments/assets/40da22d9-447d-4ad0-a500-02f862050e5c" />

Done!

<img width="400" alt="IMG_7930" src="https://github.com/user-attachments/assets/989fb56f-dacc-4bf5-a9ab-cb1311e534e4" />

## Usage

### Flashing the Firmware

You'll need [VS Code](https://code.visualstudio.com/) with the [PlatformIO IDE extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) installed. Once installed, restart VS Code, open the repository folder, and dependencies will pull in automatically.

Plug the board in via USB-C, then hit the upload button (→) in the bottom status bar. If the board doesn't reboot with the new firmware automatically, hold the BOOT button on the back and press RESET once, then release BOOT.

The board should auto-detect, but if you hit an upload failure, check that the correct board is selected in the status bar. If it still won't upload, try:

- Disconnect and reconnect the USB cable
- Check that your cable supports data transfer (some USB-C cables are charge-only)
- Try a different USB port on your computer

Read more about PlatformIO [here](https://docs.platformio.org/en/latest/).

### First Boot

On first boot, the radar broadcasts a WiFi hotspot called `MicroRadar-Setup`. Connect to it from your phone or laptop and a configuration page will appear automatically (or go to your browser if it doesn't). Enter your WiFi credentials and hit save. The board will restart and connect to your network.

If the hotspot doesn't appear straight away, give it a moment. If it still hasn't appeared after 30 seconds, exit the WiFi settings on your device and go back in to force a refresh. It'll usually show up then.

### Configuration

Once connected to your network, the radar config is accessible at [http://microradar.local](http://microradar.local) from any device on the same network.

Here you can set:

- **Location** (latitude and longitude): the centre point of your radar
- **Radar radius**: how wide the scan extends (in degrees, 2 degrees is the limit to avoid rate limiting)
- **Display options**: toggle visual elements
- **OpenSky credentials**: your client ID and secret (if you've made an account - again, highly recommend!)

<img width="400" alt="image" src="https://github.com/user-attachments/assets/45e6219c-2672-4197-baad-16ae08180b58" />

If you've made an OpenSky account (which I highly recommend), you can find your credentials under your account settings at opensky-network.org. With authentication, you get 4000 requests per day instead of 400, making the live view much more accurate. Read more about the API [here](https://opensky-network.org).

This configuration page is accessible anytime the device is connected to WiFi, so you can tweak settings whenever you want.

That's it! Once you've configured everything, you should see a live view of all flights over your location. Enjoy :)

<img width="400" alt="IMG_7935" src="https://github.com/user-attachments/assets/118b9a1c-c2c0-488d-b638-d8684a30b1d7" />

## Notes

> Designed and developed as part of a wedding present for a mate who loves aviation (congratulations to both him and his wife!)

> Inspired by [therealhacksaw](https://www.instagram.com/therealhacksaw/)'s desk radar

> Built with ♥︎ in London
