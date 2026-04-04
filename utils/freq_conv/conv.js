// Based on neologix's code from http://gendev.spritesmind.net/forum/viewtopic.php?f=24&t=1882

util = require("util");

NTSC_YM2612_HZ = 7670454;
PAL_YM2612_HZ = 7600489;
NTSC_PSG_HZ = 3579545;
PAL_PSG_HZ = 3546895;
NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];

function MIDI_noteToHz(n) {
  return 440 * Math.pow(2, (n - 69) / 12);
}

function MIDI_noteToName(n) {
  return NOTE_NAMES[n % 12] + (Math.floor(n / 12) - 1);
}

function SN_hzToFnum(clock, hz) {
  var div = 0.0625,
    reg = 0.5;
  if (hz <= 0) throw new RangeError("hzToFnum - cannot divide by <= 0");
  return ((div * reg * clock) / hz) | 0; // x|0 is the same as casting as int
}

function YM_fnumToHz(clock, fn, b, rs = 1, ch = 6, fm = 24) {
  var cl = ch * fm,
    pre = clock / (rs * cl);
  return fn * pre * Math.pow(2, b - 21);
}

function YM_hzToFnum(clock, hz, dp = 4, rs = 1, ch = 6, fm = 24) {
  var cl = ch * fm,
    pre = clock / (rs * cl),
    t = Math.pow(10, dp);
  var fn = (hz * (1 << 14)) / pre,
    b = 8;
  while (
    fn < 2048 &&
    ((hz * t) | 0) / t != ((YM_fnumToHz(clock, fn | 0, b, rs, ch, fm) * t) | 0) / t &&
    --b > 0
  )
    fn *= 2;
  fn >>= 1;
  return { fnum: fn | 0, block: b };
}

var systems = [
  { name: "PAL", ym2612Hz: PAL_YM2612_HZ, psgHz: PAL_PSG_HZ },
  { name: "NTSC", ym2612Hz: NTSC_YM2612_HZ, psgHz: NTSC_PSG_HZ },
];

systems.forEach((sys) => {
  console.log("\n" + sys.name + " YM2612 frequency table\n");
  for (var note = 0; note <= 106; note++) {
    hz = MIDI_noteToHz(note);
    f = YM_hzToFnum(sys.ym2612Hz, hz, 1, 4);
    console.log({
      note,
      name: MIDI_noteToName(note),
      hz: hz.toFixed(1),
      fnum: f.fnum,
      block: f.block > 0 ? f.block - 1 : f.block,
    });
  }
});

systems.forEach((sys) => {
  console.log("\n" + sys.name + " PSG frequency table\n");
  for (var note = 45; note <= 127; note++) {
    hz = MIDI_noteToHz(note);
    f = SN_hzToFnum(sys.psgHz, hz);
    console.log({
      note,
      name: MIDI_noteToName(note),
      hz: hz.toFixed(1),
      f,
    });
  }

  process.stdout.write("static const u16 TONES[NUM_FREQUENCIES] = { ");
  for (var note = 45; note <= 127; note++) {
    hz = MIDI_noteToHz(note);
    f = SN_hzToFnum(sys.psgHz, hz);
    process.stdout.write(f + ", ");
  }
  process.stdout.write("};\n");
});
