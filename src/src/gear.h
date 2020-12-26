#pragma once



namespace gear {
  struct Jump {
    //uint16_t count;
    int32_t count;
    int32_t delta;
    int error;
  };

  // encoder counts where we have our next and previous jumps
  struct Jumps{
    int next;
    int prev;
    int last;
  };


  // need a set nominator/denominator that checks to ensure the slope < 1 for bresenham
  
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"  
  // Narrowing comes due to integer promotion in arithmetic operations
  // k, the encoder count delta for the next step pulse, should fit within a short integer

  inline Jump next_jump_forward(int d, int n, int e, int32_t count) {
    // k = encoder_pulses per step
    int32_t k = (d - 2 * e + 2 * n - 1) / (2 * n);
    return {count + k, k, e + k * n - d};
  }

  inline Jump next_jump_reverse(int d, int n, int e, int32_t count) {
    // k = encoder_pulses per step
    int32_t k = 1 + ((d + 2 * e) / (2 * n));
    return {count - k, k, e - k * n + d};
  }

  struct State {
    int D, N; // pulse ratio : N/D
    //int err = 0;
    int nerror = 0;
    int perror = 0;
    // what units is this, seems to be encoder tics
    int output_position = 0;
    Jumps jumps = {0,0};
    bool is_setting_dir = false;

    void calc_jumps(int encoder_count,bool dir){
      Jump nx = next_jump_forward(D,N,nerror,encoder_count);
      Jump px = next_jump_reverse(D,N,perror,encoder_count); 
      jumps.next =  nx.count;
      jumps.last = encoder_count;
      nerror = nx.error;
      if(dir){
          // if we change the direction the "previous" jump will be the current position
          jumps.prev = px.count;
          //jumps.prev = encoder_count;
          perror = nx.error + D -N;
      }
      // this doesn't seem to work for !dir 
      else{
        //jumps.next = encoder_count;
        jumps.prev = px.count + 1;
        //jumps.prev = px.count;
        perror = nx.error + D +N;
      }


    }
    bool setRatio(int nom, int den){
      if (nom > den){
        return false;
      }
      D = den;
      N = nom;
      return true;
    }
    
  };

  

  /*  not using this yet
  Range range;

  template <typename RationalNumber>
  void configure(const RationalNumber& ratio, int32_t start_position) {
    state.D = ratio.denominator();
    state.N = ratio.numerator();
    state.nerror = 0;
    state.perror = 0;
    range.next = next_jump_forward(ratio.denominator(), ratio.numerator(), 0, start_position);
    range.prev = next_jump_reverse(ratio.denominator(), ratio.numerator(), 0, start_position);
  }
  
  inline unsigned phase_delay(int32_t input_period, int e) {
    if (e < 0) e = -e;
    return (input_period * e) / (state.N);
  }
  */
  
}