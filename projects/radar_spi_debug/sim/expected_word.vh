// Independent wire-format oracle: do not call the DUT header/pattern functions.
function [15:0] expected_word;
    input integer seq,position;
    integer beam,j,idx; reg [15:0] h[0:15];
    begin
        beam=(9+seq)%19;
        h[0]=16'hA610;h[1]=3;h[2]=16;h[3]=beam;
        h[4]=(beam-9)*500;h[5]=19;h[6]=POINTS;
        h[7]=CHIRPS-1;h[8]=CHIRPS;h[9]=4;
        h[10]=16'hA286;h[11]=1;h[12]=BYTES;h[13]=32;h[14]=seq;
        h[15]=0;for(j=0;j<15;j=j+1)h[15]=h[15]^h[j];
        idx=POINTS+(position-16)/2;
        if(position<16)expected_word=h[position];
        else if(position%2==0)expected_word=idx;
        else expected_word=16'h8000|seq;
    end
endfunction
