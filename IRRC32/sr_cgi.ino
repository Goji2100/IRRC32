// sr_cgi (c) Goji2100
//------------------------------------------------------------------------------
#define SR_LOCAL  0   // Internal call
#define SR_REMOTE 1   // Remote call
int    sr_root = SR_LOCAL;


#define SR_FILEINC_MAX 5
int    sr_curr_fileinc = 0;  // Include nest
File   sr_curr_file    [SR_FILEINC_MAX];  // File Handles
String sr_curr_filename[SR_FILEINC_MAX];  // File names
int    sr_curr_fileline[SR_FILEINC_MAX];  // Current line #


#define SR_WORDS_MAX 64  // define words
int    sr_curr_words = 0;
String sr_wordn[SR_WORDS_MAX];
String sr_wordv[SR_WORDS_MAX];


#define SR_DEVS_MAX 32   // device names
String sr_room = "";          // room : "Study" | "Living" | "Bedroom" ..
int    sr_curr_devs = 0;
String sr_devsn[SR_DEVS_MAX]; // DEV_NAME : "TV" | "CL" | "AC" ..
String sr_devsm[SR_DEVS_MAX]; // DEV_MAPS : map_start
String sr_devst[SR_DEVS_MAX]; // DEV_TEMP : temperature setting script


#define IF_NEST_MAX 32
int sr_ifdefn = 0;
int sr_ifdef[IF_NEST_MAX];


char bbuff[128]; // base64 read buffer   > 40 * 3
char dbuff[180]; // base64 encode buffer > ((40 * 3) * 8 / 6)
String strBuff;


void sr_begin() {
  sr_root = SR_LOCAL;
  sr_CGI("index.htm");
}


void handleCGI() {
  sr_root = SR_REMOTE;
  sr_CGI((server.hasArg("rc") ? server.arg("rc") : "home.htm"));
}


void sr_CGI(String rc_name) {
  String ibuff, obuff;
  strBuff.reserve(1600);


  if (rc_name.indexOf('.') == -1) rc_name += ".htm";
  if (rc_name[0] != '/') rc_name = "/" + rc_name;
  sr_curr_filename[sr_curr_fileinc = 0] = rc_name;
  if (!SPIFFS.exists(sr_curr_filename[sr_curr_fileinc]) ||
      !(sr_curr_file[sr_curr_fileinc] = SPIFFS.open(sr_curr_filename[sr_curr_fileinc], "r"))) {
    if (sr_root == SR_REMOTE)
      server.send(404, "text/plain", "File Not Found / File Open Error :" + sr_curr_filename[sr_curr_fileinc]);
    else
      DBG_OUTPUT_PORT.println("File Not Found / File Open Error :" + sr_curr_filename[sr_curr_fileinc]);
    return;
  }


  DBG_OUTPUT_PORT.println("handleFileRead: " + rc_name);
  if (sr_root == SR_REMOTE) {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    WiFiClient client = server.client();
  }


  sr_ifdefn = 0;
  sr_curr_words = 0;


  while (1) {
    while (sr_curr_file[sr_curr_fileinc].available()) {
      ibuff = sr_curr_file[sr_curr_fileinc].readStringUntil('\n');
      sr_curr_fileline[sr_curr_fileinc]++;
      ibuff.replace("\r", "");
      //DBG_OUTPUT_PORT.println("1[" + sr_curr_filename[sr_curr_fileinc] + ":" + String(sr_curr_fileline[sr_curr_fileinc]) + "] " + ibuff);


      // #if
      // #else
      // #endif
      //----------------------------------------------------------------------------
      if (sr_ifdefn) {
        if (ibuff.substring(0, 6) == "#endif") {
          --sr_ifdefn;
          continue;
        }


        if (sr_ifdef[sr_ifdefn] >= 2) {  // watch "#else"
          if (ibuff.substring(0, 5) == "#else") {
            sr_ifdef[sr_ifdefn] = ((sr_ifdef[sr_ifdefn] == 2) ? 1 : 0);
            continue;
          }
        }


        if (!(sr_ifdef[sr_ifdefn] & 1)) { // nested "#if" in false
          if (ibuff.substring(0, 4) == "#if ") {
            if (sr_ifdefn < (IF_NEST_MAX - 1)) {
              sr_ifdef[++sr_ifdefn] = 0;
              continue;
            } else ibuff = sr_error("#if nest over"); // (^^;
          } else
            continue;
        }
      }


      if (ibuff[0] != '#') {
        sr_sendText((ibuff + "\r\n"), 0);
        continue;
      }


      // #// comment
      //----------------------------------------------------------------------------
      if (ibuff.substring(0, 3) == "#//") continue;


      // # word replace
      //----------------------------------------------------------------------------
      obuff = sr_replaceWord(ibuff);
      //DBG_OUTPUT_PORT.println(obuff);
      if (obuff == "") continue;


      // #if #abc == '123' => true:'#?=' / false:'#?!'
      //----------------------------------------------------------------------------
      if (obuff.substring(0, 2) == "#?") {
        if (sr_ifdefn < (IF_NEST_MAX - 1)) {
          sr_ifdef[++sr_ifdefn] = (2 + ((obuff[2] == '=') ? 1 : 0));
          continue;
        }
        sr_sendText(sr_error("#if nest over"), 0);
        continue;
      }


      // #<a       #include <a>
      //----------------------------------------------------------------------------
      if (obuff.substring(0, 2) == "#<") {
        obuff = sr_include(obuff);
      } else {
        if (obuff[0] == '#') obuff[0] = ' ';
        sr_sendText((obuff + "\r\n"), 0);
      }
    }
    if (!sr_curr_fileinc) break;
    sr_curr_file[sr_curr_fileinc].close();
    sr_curr_fileinc--;
    sr_sendText("", 1);
  }
  sr_curr_file[sr_curr_fileinc].close();
  if (sr_ifdefn)
    DBG_OUTPUT_PORT.println("! not found #endif (" + String(sr_ifdefn) + "):" + sr_curr_filename[sr_curr_fileinc]);
  sr_sendText("", 2);
  strBuff = "";
  sr_dispRoom();
}


void sr_dispRoom() {
  DBG_OUTPUT_PORT.println("room = " + sr_room);
  for (int i = 0; i < sr_curr_devs; i++)
    DBG_OUTPUT_PORT.println(sr_devsn[i] + "[" + sr_devsm[i] + ", " + sr_devst[i] + "]");
}


int sr_handleMQTT(String cmnd) {
  // "Study/TV=ON" | "Bedroom/AC=H25"
  String room = "", dev = "", val = "", buff = "", reps = "";
  int p = cmnd.indexOf("/");
  if (p != -1) {
    room = cmnd.substring(0, p);
    cmnd = cmnd.substring(p + 1);
  }
  p = cmnd.indexOf("=");
  if (p != -1) {
    dev = cmnd.substring(0, p);
    val = cmnd.substring(p + 1);
  }
  DBG_OUTPUT_PORT.println("room=" + room + ", dev=" + dev + ", val=" + val);
  if ((room != "") && (room != sr_room)) return (-1); // ERROR different room
  for (p = 0; p < sr_curr_devs; p++) if (sr_devsn[p] == dev) break;
  if (p >= sr_curr_devs) return (-2); // ERROR not found device
  int q = sr_devsm[p].toInt();
  if (dev == "TV") {
    if ((val == "ON") || (val == "OFF")) {
      buff = String(q + 16);
    } else {
      for (int i = 0; i < val.length(); i++) {
        char w = val.charAt(i);
        if (!isdigit(w)) return (-3); // ERROR invalid channel
        buff += (String(q + 3 + (w - '0')) + "|");
      }
    }


  } else if (dev == "CL") {
    if      (val == "ON" ) buff = String(q + 5);
    else if (val == "OFF") buff = String(q + 6);
    else if (val == "ECO") buff = String(q + 1);
    else if (val == "MIN") buff = String(q + 2);
    else return (-4); // ERROR DEV_TEMP Invalid format


  } else if (dev == "AC") {
    int r = sr_devst[p].indexOf("|");
    if (r == -1) return (-4); // ERROR DEV_TEMP Invalid format
    String calc = sr_devst[p].substring(0, r);
    int s = sr_devst[p].indexOf("|", r + 1);
    if (s == -1) return (-5); // ERROR DEV_TEMP Invalid format
    String temp = sr_devst[p].substring(r + 1, s);
    String csum = sr_devst[p].substring(s + 1);
    DBG_OUTPUT_PORT.println("calc=" + calc + ", temp=" + temp + ", csum=" + csum);
    if (val == "OFF") {
      buff = String(q + 9);
      reps = csum;
    } else {
      String w = val.substring(0, 1);
      if ((w == "C") || (w == "H")) {
        buff = String(q + ((w == "C") ? 7 : 8));
        int x = val.substring(1).toInt();
        int xx = 0;
        for (int i = 0; i < calc.length(); ) {
          unsigned char vv[3];
          vv[0] = calc.charAt(++i);
          vv[1] = calc.charAt(++i);
          int v = (((vv[0] == 'x') && (vv[1] == 'x')) ? x : hex2bin(vv));
          switch (calc.charAt(++i - 3)) {
            case '=': xx  = v; break;
            case '-': xx -= v; break;
            case '+': xx += v; break;
            case '/': xx /= v; break;
            case '*': xx *= v; break;
            default: return (-6); // ERROR Invalid expression
          }
        }
        String yy = String(0x100 + xx, HEX).substring(1);
        temp.replace("xx", yy);
        temp.replace("|", "");
        reps = temp + csum;
      } else return (-7); // ERROR Invalid command (Hot | Cool)
    }
  } else return (-9); // ERROR Not found device
  ir_send(buff, reps);
  return (0);
}


void sr_sendText(String buff, int flush_sw) {
  if (sr_root == SR_LOCAL) {
    DBG_OUTPUT_PORT.print(buff);
    strBuff = "";
    return;
  }
  //DBG_OUTPUT_PORT.println("w[" + buff.substring(0, buff.length() - 2) + "] ");
  if ((strBuff.length() + buff.length()) > 1400) {
    server.sendContent(strBuff);
    strBuff = "";
  }
  strBuff += buff;
  if ((flush_sw >= 1) && (strBuff.length() != 0)) {
    server.sendContent(strBuff);
    strBuff = "";
  }
  if (flush_sw == 2) {
    server.sendContent("");
    strBuff = "";
  }
}


String sr_include(String buff) {
  int ix, jx = 0, kx = 0, rc;


  sr_sendText("", 1);


  if (sr_curr_fileinc >= (SR_FILEINC_MAX - 1))
    return (sr_error("Nest over \"" + String(sr_curr_fileinc) + "\""));


  String dataType = "";
  String filename = buff.substring(2);


  filename.trim();
  if (filename[0] != '/') filename = ("/" + filename);
  if (filename.endsWith(":base64")) {
    filename.replace(":base64", "");
    size_t ext = filename.lastIndexOf(".");
    if (ext != -1) {
      if      (filename.substring(ext) == ".bmp") dataType = "bmp";
      else if (filename.substring(ext) == ".png") dataType = "png";
      else if (filename.substring(ext) == ".gif") dataType = "gif";
      else if (filename.substring(ext) == ".jpg") dataType = "jpeg";
    }
  } else if (((ix = filename.indexOf(':')) != -1) &&
             (filename.substring(ix, ix + 8) == ":irdata:")) {
    if ((kx = filename.indexOf(':', ix + 8)) != -1) {
      jx = filename.substring(ix + 8).toInt();
      kx = filename.substring(kx + 1).toInt();
      if ((jx > 0) && (kx > 0) && ((jx + kx) <= IR_TLEN)) {
        dataType = "irdata";
        filename = filename.substring(0, ix);
      } else return (sr_error("DBFILE parameter incorrect - 2 \"" + filename + "\""));
    } else return (sr_error("DBFILE parameter incorrect - 1 \"" + filename + "\""));
  } else dataType = "";


  if (!SPIFFS.exists(filename) || !(sr_curr_file[sr_curr_fileinc + 1] = SPIFFS.open(filename, "r")))
    return (sr_error("File not found \"" + filename + "\""));


  DBG_OUTPUT_PORT.print("#include: " + filename);
  DBG_OUTPUT_PORT.println(":" + dataType);
  sr_curr_fileinc++;
  sr_curr_fileline[sr_curr_fileinc] = 0;
  sr_curr_filename[sr_curr_fileinc] = filename;
  if (dataType == "") return ("");


  if (dataType == "irdata") {
    DBG_OUTPUT_PORT.println(filename + ":" + dataType + ":" + jx + ":" + kx);
    while (sr_curr_file[sr_curr_fileinc].available()) {
      String ibuff = sr_curr_file[sr_curr_fileinc].readStringUntil('\n');
      ix = sr_curr_fileline[sr_curr_fileinc]++;
      ibuff.replace("\r", "");
      if (ix <= kx) {
        if ((rc = ir_dataRewrite((jx + ix), ibuff)) < 0)
          return (sr_error("File i/o error (" + String(rc) + ") - \"" + filename + "\""));
        if (rc == 1) DBG_OUTPUT_PORT.println(String(jx + ix) + ":w:" + ibuff);
      } else
        DBG_OUTPUT_PORT.println(String(jx + ix) + ":x:" + ibuff);
    }
  } else {
    sr_sendText("\'data:image/" + dataType + ";base64,\'\r\n", 0);
    while (sr_curr_file[sr_curr_fileinc].available()) {
      size_t b64len = sr_curr_file[sr_curr_fileinc].readBytes(bbuff, 3 * 40);
      sr_base64Encode(bbuff, dbuff, b64len);
      if (sr_root == SR_REMOTE)
        sr_sendText("+\'" + String(dbuff) + "\'\r\n", 0);
    }
    sr_sendText(";\r\n", 0);
  }
  sr_curr_file[sr_curr_fileinc].close();
  sr_curr_fileinc--;
  return ("");
}


const char base64tbl[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//------------------------------------------------------------------------------
void sr_base64Encode(char *src, char *dst, size_t len) {
  int i, j, k;


  while (len) {
    unsigned long l3 = 0;
    for (i = 0, j = ((len > 3) ? 3 : len); i < j; i++, len--) l3 |= *src++ << (8 * (2 - i));
    for (k = 0, ++j; k < 4; k++)
      *dst++ = ((k < j) ? (base64tbl[(l3 >> (18 - (k * 6))) & 0x3F]) : ('='));
  }
  *dst = '\0';
}


int sr_wspSkip(String s, int sx, int lp) {
  while ((sx < lp) && (isWhitespace(s[sx]))) sx++;
  return (sx);
}


int sr_wordEnd(String s, int sx, int lp) {
  while ((sx < lp) && (isAlphaNumeric(s[sx]) || (s[sx] == '_'))) sx++;
  return (sx);
}


String RsvWord = "room DEV_NAME DEV_MAPS DEV_TEMP ";


int sr_wordNameAdd(String w, String v) {
  if (w == "room") {
    sr_room = v;
    sr_curr_devs = 0;
  } else if (w == "DEV_NAME") {
    if (sr_curr_devs < SR_DEVS_MAX) sr_devsn[sr_curr_devs++] = v;
    sr_devsm[sr_curr_devs - 1] = "";
    sr_devst[sr_curr_devs - 1] = "";
  } else if (w == "DEV_MAPS") {
    if (sr_curr_devs) sr_devsm[sr_curr_devs - 1] = v;
  } else if (w == "DEV_TEMP") {
    if (sr_curr_devs) sr_devst[sr_curr_devs - 1] = v;
  } else {
    int i;
    for (i = 0; i < sr_curr_words; i++) if (sr_wordn[i] == w) break;
    if (i >= sr_curr_words) {
      if ((i + 1) >= SR_WORDS_MAX) return (-1); // table over
      sr_curr_words++;
      sr_wordn[i] = w;
    }
    sr_wordv[i] = v;
    return (i);
  }
  //  DBG_OUTPUT_PORT.print("add " + w + " = " + v);
  //  DBG_OUTPUT_PORT.printf(" > %d\n", sr_curr_devs);
  return (0);
}


int sr_wordValsGet(String w) {
  for (int i = 0; i < sr_curr_words; i++)
    if (sr_wordn[i] == w) return (i);
  return (-1);
}


String sr_error(String e) {
  DBG_OUTPUT_PORT.println("! sr_error - " + sr_curr_filename[sr_curr_fileinc] + ":" +
                          String(sr_curr_fileline[sr_curr_fileinc]) + " - " + e);
  return ("</script></font><font color=red>! sr_error - " + sr_curr_filename[sr_curr_fileinc] + ":" +
          String(sr_curr_fileline[sr_curr_fileinc]) + " - " + e + "</font><font color=black></br><script>");
}


String sr_replaceWord(String ibuff) {
  int ix, jx, kx, lx, mx, wx;


  // #if #a == b
  //----------------------------------------------------------------------------
  ix = 1;
  mx = ibuff.length();
  wx = 0;
  String sw = "", cv = "", lw = "";
  if (ibuff.substring(0, 4) == "#if ") {
    if ((ix = ibuff.indexOf('#', 4)) != -1) {
      jx = sr_wordEnd(ibuff, ++ix, mx);
      sw = ibuff.substring(ix, jx);
      if ((ix = sr_wspSkip(ibuff, jx, mx)) < (mx - 3)) {
        cv = ibuff.substring(ix, ix + 2);
        if (((cv == "==") || (cv == "!=")) &&
            ((jx = sr_wspSkip(ibuff, (ix + 2), mx)) < mx)) {
          if ((ibuff[jx] == '\'') || (ibuff[jx] == '\"')) {
            kx = ibuff.indexOf(ibuff[jx], (jx + 1));
            jx++;
          } else
            kx = sr_wordEnd(ibuff, jx, mx);
          if (kx != -1) {
            lw = (((wx = sr_wordValsGet(sw)) != -1) ? sr_wordv[wx] : "");
            if ((lw == ibuff.substring(jx, kx)))
              return ((cv == "==") ? "#?=" : "#?!");
            else
              return ((cv == "!=") ? "#?=" : "#?!");
          }
        }
      }
    }
    return (sr_error("syntax error -" + sw + " " + cv + " " + lw));
  }


  // Replace string --
  // #a = '23'
  // # s='#a';     s='23';
  // # t='#a?78';  t='23';
  // # u='#b?89';  u='89';
  //   v='#b?45';  v='#b?45';
  // # c=#F8F8F8   c=#F8F8F8
  //----------------------------------------------------------------------------
  lx = 1;
  while ((lx = ibuff.indexOf('#', lx)) != -1) {
    ix = lx;
    mx = ibuff.length();
    jx = sr_wordEnd(ibuff, ++ix, mx);
    String sw = ibuff.substring(ix, jx);
    wx = sr_wordValsGet(sw);
    String rw = (((wx != -1) && (sr_wordv[wx] != "")) ? sr_wordv[wx] : "");
    if ((jx < mx) && (ibuff[jx] == '.')) {
      sw += ".";
      jx++;
    }
    if ((jx < mx) && (ibuff[jx] == '?')) {
      sw += "?";
      if (rw != "") {
        if ((ibuff[jx + 1] == '\'') || (ibuff[jx + 1] == '\"'))
          kx = ibuff.indexOf(ibuff[jx + 1], (jx + 2)) + 1;
        else
          kx = sr_wordEnd(ibuff, (jx + 1), mx);
        sw += ibuff.substring((jx + 1), ((kx <= 0) ? mx : kx));
      }
    } else if (wx == -1) {
      lx++;
      continue;
    }
    ibuff[lx] = '\t';
    ibuff.replace("\t" + sw, rw);
  }
  if ((ibuff[1] == ' ') || (ibuff[1] == '<')) return (ibuff);


  // #a = b    #define a = b
  //----------------------------------------------------------------------------
  String sr_opes = "(+-*/)";
  jx = sr_wordEnd(ibuff, ix = 1, mx = ibuff.length());
  if (((kx = sr_wspSkip(ibuff, jx, mx)) < (mx - 1)) &&
      (ibuff[kx++] == '=') &&
      ((kx = sr_wspSkip(ibuff, kx, mx)) < mx)) {


    if ((ibuff[kx] == '\'') || (ibuff[kx] == '\"')) {
      if ((lx = ibuff.indexOf(ibuff[kx], (kx + 1))) == -1) return (sr_error("invalid syntax"));
      if (ibuff.substring(ix, jx) == "DB_FILE") {
        ir_path = ibuff.substring((kx + 1), lx);
        return ("");
      }
      return ((sr_wordNameAdd(ibuff.substring(ix, jx), ibuff.substring((kx + 1), lx)) != -1) ? "" : sr_error("table over"));
    }


    long xx[8]; char cc[8]; int xp = 0;
    for (xx[0] = cc[0] = wx = lx = 0; kx < mx; kx++) {
      if ((lx == 0) && (ibuff[kx] == ' ')) kx = sr_wspSkip(ibuff, kx, mx);
      if (isDigit(ibuff[kx])) {
        xx[xp] *= 10;
        xx[xp] += (ibuff[kx] - '0');
        lx = 1;
        if (kx < (mx - 1)) continue;
        lx = 2;
      }
      if ((kx < (mx - 1)) && (ibuff[kx] == ' ')) kx = sr_wspSkip(ibuff, kx, mx);
      while (xp) {
        switch (sr_opes[cc[xp]]) {
          case '(':
          case '+': xx[xp - 1] += xx[xp]; break;
          case '-': xx[xp - 1] -= xx[xp]; break;
          case '*': xx[xp - 1] *= xx[xp]; break;
          case '/': xx[xp - 1] /= xx[xp]; break;
        }
        if (cc[xp--] == '(') continue;
        if ((kx < mx) && (ibuff[kx] == ')')) {
          if (kx < (mx - 1)) kx = sr_wspSkip(ibuff, kx, mx);
          continue;
        }
      }
      if (kx >= (mx - 1)) break;
      if (sr_opes.indexOf(ibuff[kx]) == -1) return (sr_error("invalid syntax"));
      cc[++xp] = sr_opes.indexOf(ibuff[kx]);
      xx[xp] = 0;
      lx = 0;
    }
    if (xp != 0) return (sr_error("invalid syntax"));
    return ((sr_wordNameAdd(ibuff.substring(ix, jx), String(xx[0])) != -1) ? "" : sr_error("table over"));
  } else
    return (ibuff);
}
