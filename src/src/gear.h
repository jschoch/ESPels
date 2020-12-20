#pragma once



namespace gear {
  struct Jump {
    uint16_t count;
    uint16_t delta;
    int error;
  };

  // encoder counts where we have our next and previous jumps
  struct Jumps{
    int next;
    int prev;
  };

  
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"  
  // Narrowing comes due to integer promotion in arithmetic operations
  // k, the encoder count delta for the next step pulse, should fit within a short integer

  inline Jump next_jump_forward(int d, int n, int e, uint16_t count) {
    // k = encoder_pulses per step
    uint16_t k = (d - 2 * e + 2 * n - 1) / (2 * n);
    return {count + k, k, e + k * n - d};
  }

  inline Jump next_jump_reverse(int d, int n, int e, uint16_t count) {
    // k = encoder_pulses per step
    uint16_t k = 1 + ((d + 2 * e) / (2 * n));
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
    
  };

  volatile State state;

  void calc_jumps(int encoder_count,bool dir){
      int d = state.D, n = state.N;
      if(dir){

      }else{

      }
      Jump nx = next_jump_forward(d,n,state.nerror,encoder_count);
      Jump px = next_jump_reverse(d,n,state.perror,encoder_count); 
      state.jumps.next =  nx.count;
      state.nerror = nx.error;
      if(dir){
          state.jumps.prev = px.count ;
          state.perror = nx.error + d -n;
      }else{
        state.jumps.prev = px.count + 1;
        state.perror = nx.error + d +n;
      }


    }

  struct Range {
    Jump next{}, prev{};

    const char* next_jump(bool dir, uint16_t count) {
      return "THIS IS BORKED NOW, I changed next_jump_reverse\n";
      /*
      int d = state.D, n = state.N, e = state.err;
      if (!dir) {
        next = next_jump_forward(d, n, e, count);
        prev = {count - 1, 1u, next.error + d - n};
      } else {
        next = next_jump_reverse(d, n, e, count);
        prev = {count + 1, 1u, next.error - d + n};
      }
      */
    }
    
  };
#pragma GCC diagnostic pop

  Range range;

  template <typename RationalNumber>
  void configure(const RationalNumber& ratio, uint16_t start_position) {
    state.D = ratio.denominator();
    state.N = ratio.numerator();
    state.nerror = 0;
    state.perror = 0;
    range.next = next_jump_forward(ratio.denominator(), ratio.numerator(), 0, start_position);
    range.prev = next_jump_reverse(ratio.denominator(), ratio.numerator(), 0, start_position);
  }
  
  inline unsigned phase_delay(uint16_t input_period, int e) {
    if (e < 0) e = -e;
    return (input_period * e) / (state.N);
  }
  
}