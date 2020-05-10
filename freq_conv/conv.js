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
  // defaults to 7670448 NTSC clock value, 6 channels, 24 as the divisor
  var rs = 1,
    clock = 7670448,
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
    clock = 7670448,
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

console.log(MIDI_noteToHz(69));
