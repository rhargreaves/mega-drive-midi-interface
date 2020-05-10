// Based on neologix's code from http://gendev.spritesmind.net/forum/viewtopic.php?f=24&t=1882

util = require("util");

NTSC_YM2612_HZ = 7670454;
PAL_YM2612_HZ = 7600489;
NTSC_PSG_HZ = 3579545;
PAL_PSG_HZ = 3546895;

function MIDI_noteToHz(n) {
  return 440 * Math.pow(2, (n - 69) / 12);
}

function MIDI_HzToNote(hz) {
  var div = hz / 440.0;
  return (12 * Math.log(div)) / Math.log(2);
}

function SN_fnumToHz(clock, fn) {
  // 3579545 is a good NTSC clock value
  if (fn <= 0) throw new RangeError("FnumToHz - cannot divide by <= 0");
  var div = 0.0625,
    reg = 1 / ((fn & 0x3ff) << 1);
  return div * reg * clock;
}

function SN_hzToFnum(clock, hz) {
  // 3579545 is a good NTSC clock value
  var div = 0.0625,
    reg = 0.5;
  if (hz <= 0) throw new RangeError("hzToFnum - cannot divide by <= 0");
  return ((div * reg * clock) / hz) | 0; // x|0 is the same as casting as int
}

function YM_fnumToHz(fn, b) {
  // defaults to NTSC clock value, 6 channels, 24 as the divisor
  var rs = 1,
    clock = NTSC_YM2612_HZ,
    ch = 6,
    fm = 24;
  if (arguments.length > 5) fm = arguments[5];
  if (arguments.length > 4) ch = arguments[4];
  if (arguments.length > 3) clock = arguments[3];
  if (arguments.length > 2) rs = arguments[2];
  var cl = ch * fm,
    pre = clock / (rs * cl);
  return fn * pre * Math.pow(2, b - 21);
}

function YM_hzToFnum(hz) {
  var dp = 4,
    rs = 1,
    clock = NTSC_YM2612_HZ,
    ch = 6,
    fm = 24;
  if (arguments.length > 5) fm = arguments[5];
  if (arguments.length > 4) ch = arguments[4];
  if (arguments.length > 3) clock = arguments[3];
  if (arguments.length > 2) rs = arguments[2];
  if (arguments.length > 1) dp = arguments[1];
  var cl = ch * fm,
    pre = clock / (rs * cl),
    t = Math.pow(10, dp);
  var fn = (hz * (1 << 14)) / pre,
    b = 8;
  while (
    fn < 2048 &&
    ((hz * t) | 0) / t != ((YM_fnumToHz(fn | 0, b) * t) | 0) / t &&
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
    f = YM_hzToFnum(hz, 1, 4, sys.ym2612Hz);
    console.log({
      note,
      hz,
      fnum: f.fnum,
      block: f.block > 0 ? f.block - 1 : f.block,
    });
  }
});

systems.forEach((sys) => {
  console.log("\n" + sys.name + " PSG frequency table\n");
  for (var note = 0; note <= 106; note++) {
    hz = MIDI_noteToHz(note);
    f = SN_hzToFnum(sys.psgHz, hz);
    console.log({
      note,
      hz,
      f,
    });
  }
});
