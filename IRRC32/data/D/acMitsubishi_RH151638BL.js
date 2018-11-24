  // Air Conditioner
  //-----------------------
  // Mitsubishi RH151 638BL

#   deffile: '#def_file',
#<#def_file:irdata:#MAP_START:#MAP_COUNT

    power: 0,

    temp: { val:24, min:16, max:31 },
    func: { val: 0, val0: [0x10, 0x18, 0x08], val1: [0x02, 0x06, 0x00] },
    fan:  { val: 0, vals: [0x00, 0x01, 0x02, 0x03] },
    louv: { val: 6, vals: [0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x78] },

    select: function (btn) {

      var btnv = this.cmap[btn][0];
      var rets = '';

  //      _0 _1 _2 _3 _4 _5 _6 _7 _8 _9 _A _B _C _D _E _F 10 11 block 1
  //  FR0 23 CB 26 01 00 20 1C 09 36 40 00 00 00 00 00 10 00 E0
  //                     |                                   |
  //      12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 block 2
  //  FR1 23 CB 26 01 00 20 1C 09 36 40 00 00 00 00 00 10 00 E0
  //                     |  |  |  |  |                       |
  //                     |  |  |  |  |                    11 SUM( 0..10)
  //                     |  |  |  |  |                    23 SUM(12..22)
  //                     |  |  |  |  |
  //                     |  |  |  |  xxxx x--- Louver
  //                     |  |  |  |  0100 0xxx Auto
  //                     |  |  |  |  0100 1xxx 5
  //                     |  |  |  |  0101 0xxx 4
  //                     |  |  |  |  0101 1xxx 3
  //                     |  |  |  |  0110 0xxx 2
  //                     |  |  |  |  0110 1xxx 1
  //                     |  |  |  |  0111 1xxx Swing
  //                     |  |  |  |  |
  //                     |  |  |  |  ---- -xxx Fan speed
  //                     |  |  |  |  xxxx x000 Auto
  //                     |  |  |  |  xxxx x001 1
  //                     |  |  |  |  xxxx x010 2
  //                     |  |  |  |  xxxx x011 3
  //                     |  |  |  |
  //                     |  |  |  ---- -xx- 
  //                     |  |  |  xxxx x00x Heat
  //                     |  |  |  xxxx x01x Dry
  //                     |  |  |  xxxx x11x Cool
  //                     |  |  |
  //                     |  |  ---- xxxx 
  //                     |  |  xxxx nnnn Temp - 16
  //                     |  |
  //                     |  ---x x--- 
  //                     |  xxx0 1xxx Heat
  //                     |  xxx1 0xxx Dry
  //                     |  xxx1 1xxx Cool
  //                     |
  //                     |  ---- -x-- Power
  //                     |  xxxx x1xx Full
  //                     |  xxxx x0xx Save
  //                     |
  //                     --x- ---- Power
  //                     xx0x xxxx OFF
  //                     xx1x xxxx ON
  //

      switch (btnv) {
        case 1:
          if (++this.fan.val >= this.fan.vals.length) this.fan.val = 0;
          this.fans();
          if (this.power == 0) return('');
          break;

        case 2:
          if (++this.louv.val >= this.louv.vals.length) this.louv.val = 0;
          this.louvs();
          if (this.power == 0) return('');
          break;

        case 3:
        case 4:
          if ((btnv == 3) && (this.temp.val < this.temp.max)) this.temp.val++;
          if ((btnv == 4) && (this.temp.val > this.temp.min)) this.temp.val--;
          r = document.getElementById('temp' + this.tablen);
          r.textContent = this.temp.val;
          r.style.display  = 'block';
          if (this.power == 0) return('');
          break;


        case 5:
          return('');

        case 6:
        case 7:
        case 8:
          if (this.power != 0) return(''); // Power on in power-off
          this.power = btnv;
          break;
      }

      r = document.getElementById('temp' + this.tablen);
      if (btnv == 9) { // Power off
        r.style.display  = 'none';
        this.power = 0;
        this.mode();
        rets = (this.maps + btnv);
      } else {
        r.textContent = this.temp.val;
        r.style.display  = 'block';
        this.mode();
        rets = (this.maps + this.power);
      }

      rets += '&rep=';
      rets += ('w0517DF' + ((this.power == 0) ? '00' : '20'));
      if (this.power != 0) {
        rets += ('w0618E7' + num2hex2(this.func.val0[this.power - 6]));
        rets += ('w081AF9' + num2hex2(this.func.val1[this.power - 6]));
        rets += ('w0719F0' + num2hex2(this.temp.val - this.temp.min));
        rets += ('w091B07' + num2hex2(this.louv.vals[this.louv.val]));
        rets += ('w091BF8' + num2hex2(this.fan.vals [this.fan.val]));
      }
      rets += ('910010'); // Check Sum:(0x80 | 11)(00..10)
      rets += ('A31222'); // Check Sum:(0x80 | 23)(12..22)
      return(rets);
    }
  };

#DEV_TEMP   = '=xx-10|w0719F0xx|910010A31222'

  img_i = RC_imgs.length;
  RC_imgs[img_i] = new Image();
  RC_imgs[img_i].src = 
#<#img_file:base64
