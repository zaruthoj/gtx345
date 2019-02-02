# Simulated Garmin GTX345 Transponder

This is a simulated Garmin GTX345 Transponder I designed for my home simulator. So far it's working well for me!  I've attempted to match the real unit as exactly as possible using commonly available tools and materials.

## Status

The hardware is complete and functional.  The softare is still a work in progress, though the major UI and sim connection features are done.

I've got 4 extra PCBs in hand.  Contact zaruthoj at gmail if you want one.  I'd be happy to sell them for $10 each plus shipping.

### TODO

* Add timer pages
* Add altitude alerting pages
* Add support for saving settings to EEPROM

## Assembly

* [BOM](doc/bom.md)
* Soldering (coming soon)
* [3D Printing](doc/printing.md)
* [Finishing](doc/finishing.md)
* Final Assembly (coming soon)

## Compatibility

I'm using this with P3Dv3 running the [PilotEdge](https://www.pilotedge.net/) Client and FSUIPC.  Additional work would be required to interface this with XPlane.  However, I've designed this in a modular way such that it should be straightforward to implement an XPlane interface.

## Equipment

### Required

* 3D Printer
* Soldering Iron
* 100, 200, and 400 grit sandpaper.

### Recommended

* Soldering Hot Air or Reflow (It's possible to use an iron on these SMD components, but is much harder.)
* Dremel with 100 and 200 grit sanding bits.  For sanding the recessed area around the screen.
* 100 grit diamond needle files.  For sanding the recessed area around the screen.

### Optional

* Random Orbital Sander.  This makes sanding the face and sides so much faster.
* XTC-3D 3d print coating.  If there are larger defects in the face of your print, this stuff works wonders.  I used it to fill in some areas around the circular buttons that failed to print correctly.

## Skills Required

* 3D printing
* SMD and thru hole soldering.
* Basic familiarity with uploading software to Arduinos.
