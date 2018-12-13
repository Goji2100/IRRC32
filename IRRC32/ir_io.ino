// ir_io (c) Goji2100
//------------------------------------------------------------------------------
#define DB_RLEN (128)
String ir_path = "/IRdata.txt";


#define IR_TLEN 1024
#define IR_BLEN 62
byte ir_datam[200];  // [0]:IR_MODE, [1]:len0, [2]:len1 (1 + 2 + ((1 + IR_BLEN + 1) * 2))
int  ir_dix = 1, ir_bix;


#define IR_SLEN ((1 + (IR_BLEN * 8) + 1) * 2 * 2)  // Start + (Max byte) x 8bit + Stop x 2pair x 2set
volatile uint32_t ir_times[IR_SLEN];
volatile uint16_t ir_tix;


#define IR_MODE_NEC 0
#define IR_MODE_EHA 1


#define NEC_1T  562
#define EHA_1T  (425 + 5)


#define IR_1T   (ir_1T[ir_datam[0]]    )
#define IR_2T   (ir_1T[ir_datam[0]] * 2)
#define IR_3T   (ir_1T[ir_datam[0]] * 3)


uint16_t ir_1T[2] = { NEC_1T, EHA_1T };
uint16_t ir_S1[2] = { (NEC_1T * 16), (EHA_1T *  8) }; // Start
uint16_t ir_S0[2] = { (NEC_1T *  8), (EHA_1T *  4) };
uint16_t ir_E1[2] = { (NEC_1T *  1), (EHA_1T *  1) }; // Stop
uint16_t ir_E0[2] = { (NEC_1T * 16), (EHA_1T * 30) };
uint16_t ir_R1[2] = { (NEC_1T * 16), (EHA_1T *  8) }; // Repeat
uint16_t ir_R0[2] = { (NEC_1T *  4), (EHA_1T *  8) };


int ir_LED_val = 0;


void loop(void) {
  static int i, b = 0, c, n, l = 0;
  static char cc[8];


  server.handleClient();


#ifdef USE_MQTT
  mqtt_loop();
#endif


#ifdef PIN_ir_LED
  if (ir_LED_val > 0) digitalWrite(PIN_ir_LED, (--ir_LED_val > 0));
#endif


#ifdef CONS_CMND
  if (b == 0) DBG_OUTPUT_PORT.printf("\n%d>", ir_dix);
  b = 1;
  if (!DBG_OUTPUT_PORT.available()) return;
  c = DBG_OUTPUT_PORT.read();
  DBG_OUTPUT_PORT.write(c = toupper(c));
  if (c != '\r') {
    if ((l < (8 - 1)) && (isAlphaNumeric(c))) cc[l++] = c;
    return;
  }


  DBG_OUTPUT_PORT.write('\n');
  for (i = 0, n = -1; (i < l); i++) {
    if (isdigit(cc[i]))  {
      n = ((n == -1) ? 0 : (n * 10)) + (cc[i] - '0');
    } else {
      if ((n >= 1) && (n <= IR_TLEN)) ir_dix = n; n = -1;
      switch (cc[i]) {
        case 'D': ir_dataLoad(ir_dix); ir_dataDump(ir_dix); break;
        case 'L':
          for (int m = 1; m < IR_TLEN; m++) {
            ir_dataLoad(m);
            ir_dataDump(m);
          } ir_dataLoad(ir_dix);
          break;
        case 'R': if (ir_frameRead() != 0) ir_dataConvert(); ir_dataSave(ir_dix); break;
        case 'W': ir_frameWrite(); break;
        case 'H':
          DBG_OUTPUT_PORT.println("\nConnected SSID: " + String(ssid));
          DBG_OUTPUT_PORT.print("Connected IP address: ");
          DBG_OUTPUT_PORT.println(WiFi.localIP());
          sr_dispRoom();
          DBG_OUTPUT_PORT.printf("Heap = %d\n", ESP.getFreeHeap());
          break;
      }
      delay(1);
    }
  }
  if ((n >= 1) && (n <= IR_TLEN)) ir_dix = n; n = -1;
  l = b = 0;
#endif
}


//------------------------------------------------------------------------------
void handleIR() {
  String btn = server.arg("btn"); // 1..nnn
  String exe = server.arg("exe"); // Raed / Load / Save
  String mod = server.arg("md");  // NEC / EHA
  String v0  = server.arg("v0");  // hh,hh,hh, ...
  String v1  = server.arg("v1");  // hh,hh,hh, ...
  String rep = server.arg("rep"); // replace data
  int  io_ret = 0, btnno = btn.toInt();
  char retval[256 + 2];


  retval[0] = '\0';
  DBG_OUTPUT_PORT.print("IR exe=" + exe + ", btn=" + btn + ", rep=" + rep + ", v0=" + v0 + ", v1=" + v1 + "\n");


  if (exe == "Load") {
    if ((io_ret = ir_dataString(retval, btnno)) == btnno)
      ir_dataDump(btnno);
    else
      ioError(retval, io_ret);


  } else if (exe == "Read") {
    if (ir_frameRead()) {
      ir_dataConvert();
      if ((io_ret = ir_dataSave(btnno)) == btnno) {
        if ((io_ret = ir_dataString(retval, btnno)) != btnno)
          ioError(retval, io_ret);
      } else ioError(retval, io_ret);
    } else ioError(retval, -8);


  } else if (exe == "Save") {
    ir_datam[0] = ((mod == "NEC") ? IR_MODE_NEC : IR_MODE_EHA);
    ir_datam[2] = 0;
    if ((v0.length() > (IR_BLEN * 3)) || (v1.length() > (IR_BLEN * 3))) {
      server.send(500, "text/plain", "BAD ARGS");
      return;
    }
    if ((ir_datam[1] = hex2bins(v0, &ir_datam[3])) != 0) {
      ir_datam[1] += 3;
      ir_datam[2] = hex2bins(v1, &ir_datam[ir_datam[1]]);
      ir_datam[2] += ir_datam[1];
    }
    ir_dataDump(btnno);
    if ((io_ret = ir_dataSave(btnno)) != btnno)
      ioError(retval, io_ret);


  } else if (exe == "Update") {
    if ((v0.length() != 0) && (ir_mapUpdate(v0) == 0))
      strcpy(retval, "ERR|MAP update failed");
    else if ((v1.length() != 0) && (ir_defUpdate(v1) == 0))
      strcpy(retval, "ERR|DAT update failed");


  } else if (exe == "Send") {
    int i = 0, j, k;
    while (1) {
      if ((j = k = btn.indexOf('|', i)) == -1) j = btn.length();
      btnno = btn.substring(i, j).toInt();
      if ((io_ret = ir_dataLoad(btnno)) == btnno) {
        ir_dataDump(btnno);
        if (rep != "") {
          ir_dataPoke(rep);
          ir_dataDump(btnno);
        }
        ir_frameWrite();
      } else {
        ioError(retval, io_ret);
        k = -1;
        break;
      }
      if (k == -1) break;
      i = j + 1;
      delay(20);
    }
  } else {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }
  server.send(200, "text/plain", retval);
}


void ir_send(String btn, String rep) {
  int i = 0, j, k, io_ret;


  DBG_OUTPUT_PORT.println("ir_send(\"" + btn + "\", \"" + rep + "\")");
  while (1) {
    if ((j = k = btn.indexOf('|', i)) == -1) j = btn.length();
    int btnno = btn.substring(i, j).toInt();
    if ((io_ret = ir_dataLoad(btnno)) == btnno) {
      ir_dataDump(btnno);
      if (rep != "") {
        ir_dataPoke(rep);
        ir_dataDump(btnno);
      }
      ir_frameWrite();
    } else {
      DBG_OUTPUT_PORT.printf("ir_send error = %d\n", io_ret);
      return;
    }
    if (k == -1) break;
    i = j + 1;
    delay(80);
  }
}


void ioError(char *s, int erc) {
  sprintf(s, "ERR|IR datafile access error = %d", erc);
  DBG_OUTPUT_PORT.printf("ir_send error = %d\n", erc);
}


int hex1bin(unsigned char c) {
  c = toupper(c);
  return ((c > '9') ? (c - 'A' + 10) : (c - '0'));
}


int bin1hex(unsigned char c) {
  c &= 15;
  return ((c > 9) ? (c + 'A' - 10) : (c + '0'));
}


int hex2bin(unsigned char *s)   {
  return ((hex1bin(*s) << 4) | (hex1bin(*(s + 1))));
}


void bin2hex(unsigned char *s, unsigned char v) {
  *s++ = bin1hex(v >> 4);
  *s = bin1hex(v);
}


int hex2bins(String s, unsigned char *p) {
  int c, i, j, k, l;


  //  "  7F  ,,45, 23 ,5"
  // k=00112200110011201
  // p=7F00452305
  l = s.length();
  for (i = j = k = 0; i < l; i++) {
    c = s[i];
    if ((k == 0) && (c == ' ')) continue;
    if ((k <= 1) && (isHexadecimalDigit(c))) {
      *(p + j) <<= 4;
      *(p + j) += hex1bin(c);
      k = 1;
      continue;
    }
    if ((k <= 2) && (c == ' ')) {
      k = 2;
    } else if (c == ',') {
      *(p + ++j) = 0;
      k = 0;
    } else
      break;
  }
  return ((k > 0) ? (j + 1) : (j));
}


//------------------------------------------------------------------------------
int ir_dataLoad(int rn) {
  int i, j;
  byte buff[DB_RLEN + 8];


  ir_datam[0] = ir_datam[1] = ir_datam[2] = 0;
  File file = SPIFFS.open(ir_path, "r");
  if (!file) return (-1);


  if ((file.seek((DB_RLEN * rn), SeekSet)) && (file.read(buff, DB_RLEN))) {
    for (i = j = 0; i < (DB_RLEN - 2); i += 2) {
      if (buff[i] == ' ') break;
      ir_datam[j++] = hex2bin((unsigned char *)(buff + i));
    }
  } else rn = -2;
  file.close();
  return (rn);
}


int ir_dataSave(int rn) {
  int i, j;
  byte buff[DB_RLEN + 8];


  File file = SPIFFS.open(ir_path, "r+");
  if (!file) return (-1);


  for (i = j = 0; j < (DB_RLEN - 2); i++, j += 2) {
    if (i >= ir_datam[2])
      buff[j] = buff[j + 1] = ' ';
    else
      bin2hex(buff + j, ir_datam[i]);
  }
  buff[j++] = ' '; buff[j] = '\n';
  if ((!file.seek((DB_RLEN * rn), SeekSet)) ||
      (!file.write(buff, DB_RLEN))) rn = -2;
  file.close();
  return (rn);
}


int ir_dataRewrite(int rn, String buff) {
  int ix, lx;


  if (ir_dataLoad(rn) != rn) return (-1);
#if (1)
  DBG_OUTPUT_PORT.print(String(rn) + ": :\'");
  lx = (ir_datam[2] > 40) ? 40 : ir_datam[2];
  for (ix = 0; ix < lx; ix++)
    DBG_OUTPUT_PORT.printf("%02X", ir_datam[ix]);
  DBG_OUTPUT_PORT.println("\'");
#endif


  buff.replace("\'", ""); buff.replace(",", ""); buff.trim();
  if (buff.length() > 6) {
    if (buff.length() > ((DB_RLEN * 2) - 2)) return (-3);
    lx = ((hex1bin(buff[4]) << 4) | (hex1bin(buff[5])));
    if (lx != (buff.length() / 2)) return (-2);
  } else {
    buff = "000000";
    lx = 3;
  }


  for (ix = 0; ix < lx; ix++)
    if (ir_datam[ix] != ((hex1bin(buff[ix + ix]) << 4) | (hex1bin(buff[ix + ix + 1])))) break;
  if (ix == lx) return (0); // Same data


  for (ix = 0; ix < lx; ix++)
    ir_datam[ix] = ((hex1bin(buff[ix + ix]) << 4) | (hex1bin(buff[ix + ix + 1])));


  if (ir_dataSave(rn) != rn) return (-1);
  return (1); // Rewritten
}


int ir_mapUpdate(String s) {
  int i, j, k;
  File file;
  char buff[1 + ((4 + 1) * 5) + 1 + 8]; // >28 : [   0, 123, 234, 567, 890],


  for (i = k = 0; ((j = s.indexOf('|', i)) != -1); i = ++j) {
    if (i == 0) {
      if ((j - i) < 32) {
        DBG_OUTPUT_PORT.println(s.substring(i, j));
        if (!(file = SPIFFS.open((s.substring(i, j)), "w"))) return (0);
      } else return (0);
    } else {
      if ((j - i) < 29) {
        s.substring(i, j).getBytes((unsigned char *)buff, sizeof(buff));
        strcpy(buff + (j - i), "\n");
        if (!file.write((byte *)buff, ((j - i) + 1))) break;
        k++;
      }
    }
  }
  file.close();
  return (k);
}


int ir_defUpdate(String s) {
  int j, k, m, n = 0, p;
  uint8_t buff[DB_RLEN + 8];


  if (((j = s.indexOf('|', 0)) == -1) || (j > 32) ||
      ((k = s.indexOf('|', j + 1)) == -1) ||
      ((m = s.substring(j + 1, k).toInt()) < 1) ||
      ((n = s.substring(k + 1   ).toInt()) < 1) ||
      (m > n))
    return (0);
  DBG_OUTPUT_PORT.println(s.substring(0, j));
  File file = SPIFFS.open(s.substring(0, j), "w");
  if (!file) return (0);


  for (k = p = 0; m < n; m++, p = 0) {
    if (ir_dataLoad(m) == -1) break;
    buff[p++] = '\'';
    for (int i = 0; i < ir_datam[2]; i++) {
      bin2hex(buff + p, ir_datam[i]);
      p += 2;
    }
    strcpy((char *)(buff + p), (((m + 1) >= n) ? "\'\n" : "\',\n"));
    if (!file.write((byte *)buff, strlen((char *)buff))) break;
    k++;
  }
  file.close();
  return (k);
}


void ir_dataPoke(String rep) {
  unsigned char reps[128];


  if (ir_datam[2] == 0) return;
  rep.toCharArray((char *)reps, sizeof(reps));


  for (int i = 0; reps[i]; ) {
    //DBG_OUTPUT_PORT.print(">" + rep.substring(i) + "<\n");
    int w = ((reps[i] == 'w') ? reps[i++] : 0);
    int p = hex2bin(&reps[i]); i += 2;
    int q = p;
    if (w) {
      q = hex2bin(&reps[i]); i += 2;
    }
    int m = hex2bin(&reps[i]); i += 2;
    int v = hex2bin(&reps[i]); i += 2;
    int s = (p & (0x80 | 0x40));
    //DBG_OUTPUT_PORT.printf(">%02X%02X%02X%02X<\n", p, q, m, v);
    if (s) {
      ir_datam[p = ((p & 0x3F) + 3)] = 0;
      for (int j = m; j <= v; j++)
        if (s & 0x40)
          ir_datam[p] -= ir_datam[j + 3];
        else
          ir_datam[p] += ir_datam[j + 3];
    } else {
      ir_datam[p + 3] = (ir_datam[p + 3] & m) | v;
      if (q != p)
        ir_datam[q + 3] = (ir_datam[q + 3] & m) | v;
    }
  }
}


int ir_dataString(char *s, int rn) {
  int retc = ir_dataLoad(rn);
  if (retc != rn) return (retc);
  if (ir_datam[2] == 0) {
    sprintf(s, "%d|||", rn);
    return (rn);
  }


  sprintf(s, "%d|%s|%d,%d|", rn,
          (ir_datam[0] == IR_MODE_NEC) ? "NEC" : "EHA",
          (ir_datam[1] - 3), (ir_datam[2] - 3));  //x
  int p = strlen(s);
  for (int i = 3; i < ir_datam[2]; i++) {
    sprintf(s + p, "%02X%c", ir_datam[i], (((i + 1) == ir_datam[1]) ? '|' : ',')); //x
    p += 3;
  }
  strcpy(s + p - 1, "");
  return (rn);
}


void ir_dataDump(int rn) {
  if (ir_datam[1] == 0) return;
  DBG_OUTPUT_PORT.printf("%d : %s [%d,%d]%s", rn,
                         (ir_datam[0] == IR_MODE_NEC) ? "NEC" : "EHA",
                         (ir_datam[1] - 3), (ir_datam[2] - 3),
                         (ir_datam[1] > (3 + 4)) ? "\n" : " ");
  for (int i = 3; i < ir_datam[2]; i++) {
    DBG_OUTPUT_PORT.printf("%02X%s",  ir_datam[i], ((((i + 1) == ir_datam[1]) || ((i + 1) == ir_datam[2])) ? "\n" : ", "));
  }
}


int ir_frameRead() {
  int ir_stat = LOW;  // wait IR on


  for (int i = 0; ; i++) {
    if (digitalRead(PIN_ir_in) == ir_stat) break; // IR on
    if (i >= 10000) return (0);
    delay(1);
  }


  for (ir_tix = 0; ir_tix < IR_SLEN; ir_tix++) {
    unsigned long ir_stime = micros();
    while ((digitalRead(PIN_ir_in) == ir_stat) && ((micros() - ir_stime) < 20000));
    ir_times[ir_tix] = (micros() - ir_stime);
    if (ir_tix & 1) ir_times[ir_tix] += 50; else ir_times[ir_tix] -= 50;
    if (ir_times[ir_tix] >= 20000) break;
    ir_stat = ((ir_stat == LOW) ? HIGH : LOW);
  }
  delay(1);
  return (ir_tix);
}


//#define LIRC  // LIRC format


void ir_dataConvert() {
  ir_datam[0] = ((ir_times[1] > (NEC_1T * 6)) ? IR_MODE_NEC : IR_MODE_EHA); // Leaders off length
  ir_datam[1] = 0;
  ir_datam[2] = 0;
  ir_datam[ir_bix = 3] = 0;
  for (int ix = 0, bp = 0; ix < ir_tix; ix += 2) {
    if (ir_times[ix] > (EHA_1T * 4)) {  // Start bit
      DBG_OUTPUT_PORT.printf("\n\n%s [%4d] : %4u+%4u\n", ((ir_datam[0] == IR_MODE_NEC) ? "NEC" : "EHA"),
                             ir_tix, ir_times[ix], ir_times[ix + 1]);
    } else if (ir_times[ix + 1] >= (EHA_1T * 15)) { // Stop bit
      DBG_OUTPUT_PORT.printf("%4u+%4u !\n", ir_times[ix], ir_times[ix + 1]);
      if (ir_datam[1] == 0) ir_datam[1] = ir_bix;
    } else {
      DBG_OUTPUT_PORT.printf("%4u+%4u ", ir_times[ix], ir_times[ix + 1]);
      ir_datam[ir_bix] >>= 1;
      if (ir_times[ix + 1] > IR_2T) ir_datam[ir_bix] |= 0x80;
      if (++bp >= 8) {
        DBG_OUTPUT_PORT.printf("- %02X\n", ir_datam[ir_bix]);
        ir_datam[++ir_bix] = bp = 0;
      }
    }
  }
  ir_datam[2] = ir_bix;
}


void ir_frameWrite() {
  if (ir_datam[1] == 0) return;
#ifdef PIN_ir_LED
  digitalWrite(PIN_ir_LED, ((ir_LED_val = 5000) > 0));
#endif
  for (ir_tix = 0, ir_bix = 3; ir_bix < ir_datam[2]; ) {
    if ((ir_bix == 3) || (ir_bix == ir_datam[1])) {
      ir_times[ir_tix++] = ir_S1[ir_datam[0]]; // Start on
      ir_times[ir_tix++] = ir_S0[ir_datam[0]]; //       off
    }
    for (int bp = 0x01; (bp != 0x0100); bp <<= 1) {
      ir_times[ir_tix++] = IR_1T;
      ir_times[ir_tix++] = ((ir_datam[ir_bix] & bp) ? IR_3T : IR_1T);
    }
    if (((++ir_bix) == ir_datam[1]) || (ir_bix == ir_datam[2])) {
      ir_times[ir_tix++] = ir_E1[ir_datam[0]]; // Stop on
      ir_times[ir_tix++] = ir_E0[ir_datam[0]]; //      off
    }
  }
  delay(1);


  if (ir_tix) {
    for (int i = 0; i < ir_tix; i++) {
      unsigned int  sw = ((i & 1) ? LOW : HIGH);
      unsigned long stime = micros();
      while ((stime + ir_times[i]) > micros()) {
        digitalWrite(PIN_ir_out, sw); delayMicroseconds( 8);  //  8.77us
        digitalWrite(PIN_ir_out,  0); delayMicroseconds(16);  // 17.54us = 26.31
        digitalWrite(PIN_ir_out,  0);
      }
    }
  }
  delay(1);
  DBG_OUTPUT_PORT.printf("[%4d] Send\n", ir_tix);
}
