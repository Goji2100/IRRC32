  // Air Conditioner
  //-----------------------
  // Panasonic A75C3215

#   deffile: '#def_file',
#<#def_file:irdata:#MAP_START:#MAP_COUNT

    power: 0,

    temp: { val:24, min:16, max:31 },
    func: { val: 0, vals: [0x30, 0x20, 0x40] },
    fan:  { val: 0, val0: [0xA0, 0x30, 0x30, 0x40, 0x50, 0x70], val1: [0x00, 0x20, 0x00, 0x00, 0x00, 0x00] },
    louv: { val: 0, vals: [0x0F, 0x01, 0x02, 0x03, 0x04, 0x05] },

    select: function (btn) {

      var btnv = this.cmap[btn][0];
      var rets = '';

  //      _0 _1 _2 _3 _4 _5 _6 _7
  //  FR0 02 20 E0 04 00 00 00 06
  //                           |
  //                         SUM(00..06)
  //
  //      _8 _9 _A _B _C _D _E _F 10 11 12 13 14 15 16 17 18 19 1A
  //  FR1 02 20 E0 04 00 39 32 80 AF AD 00 0E E0 40 00 81 04 1D 1D
  //                     |  |     |  |           |           |  |
  //                     |  |     |  |           |           |  SUM(08..19)
  //                     |  |     |  |           |           |
  //                     |  |     |  |           |           ---x ---- 
  //                     |  |     |  |           |           xxx0 xxxx 
  //                     |  |     |  |           |           xxx1 xxxx 
  //                     |  |     |  |           |
  //                     |  |     |  |           -x-- ---- 
  //                     |  |     |  |           x0xx xxxx 
  //                     |  |     |  |           x1xx xxxx 
  //                     |  |     |  |           |
  //                     |  |     |  |           --x- ---- FAN
  //                     |  |     |  |           xx0x xxxx Full
  //                     |  |     |  |           xx1x xxxx Silent
  //                     |  |     |  |
  //                     |  |     |  ---- xxxx Louver Left & Right
  //                     |  |     |  xxxx 1101 Auto
  //                     |  |     |  xxxx 0110 1
  //                     |  |     |  xxxx 0111 2
  //                     |  |     |  xxxx 1000 3
  //                     |  |     |  xxxx 1001 4
  //                     |  |     |  xxxx 1010 5
  //                     |  |     |  xxxx 1011 6
  //                     |  |     |  xxxx 1100 7
  //                     |  |     |
  //                     |  |     xxxx ---- FAN
  //                     |  |     1010 xxxx Auto
  //                     |  |     0011 xxxx 1
  //                     |  |     0100 xxxx 2
  //                     |  |     0101 xxxx 3
  //                     |  |     0111 xxxx 4
  //                     |  |     |
  //                     |  |     ---- xxxx Louver Up & Down
  //                     |  |     xxxx 1111 Auto
  //                     |  |     xxxx 0001 1
  //                     |  |     xxxx 0010 2
  //                     |  |     xxxx 0011 3
  //                     |  |     xxxx 0100 4
  //                     |  |     xxxx 0101 5
  //                     |  |
  //                     |  00xx xxxx 
  //                     |  xxnn nnnn (Temp * 2)
  //                     |
  //                     ---- ---x Power
  //                     xxxx xxx0 OFF
  //                     xxxx xxx1 ON
  //

      switch (btnv) {
        case 1:
          if (++this.fan.val >= this.fan.val0.length) this.fan.val = 0;
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

      if (this.power == 0) return('');
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
      rets += ('0D8E' + num2hex2((this.power == 0) ? 0 : (this.func.vals[this.power - 6] | 0x01)));
      rets += ('0EC0' + num2hex2(this.temp.val * 2));
      rets += ('1000' + num2hex2(this.fan.val0[this.fan.val] + this.louv.vals[this.louv.val]));
      rets += ('15DF' + num2hex2(this.fan.val1[this.fan.val]));
      rets += ('870006'); // Check Sum:(0x80 | 07)(00..06)
      rets += ('9A0819'); // Check Sum:(0x80 | 1A)(08..19)
      return(rets);
    }
  };

#DEV_TEMP   = '=xx*02|0EC0xx|8700069A0819'

  img_i = RC_imgs.length;
  RC_imgs[img_i] = new Image();
  RC_imgs[img_i].src = 
#<#img_file:base64
