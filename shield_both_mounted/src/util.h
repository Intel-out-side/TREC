using ll = long long;
long integerPow(long, int);

/*
   convert the String of hexadecimal representation of data to binary
   @ param data : raw data of sensor
   @ return : String which represents the sensor data in binary
*/
String hexToBin(String data) {
  String resultStr = "";
  for (int i = 0; i < 8; i++) {
    if (data.charAt(i) == '0') resultStr += "0000";
    else if (data.charAt(i) == '1') resultStr += "0001";
    else if (data.charAt(i) == '2') resultStr += "0010";
    else if (data.charAt(i) == '3') resultStr += "0011";
    else if (data.charAt(i) == '4') resultStr += "0100";
    else if (data.charAt(i) == '5') resultStr += "0101";
    else if (data.charAt(i) == '6') resultStr += "0110";
    else if (data.charAt(i) == '7') resultStr += "0111";
    else if (data.charAt(i) == '8') resultStr += "1000";
    else if (data.charAt(i) == '9') resultStr += "1001";
    else if (data.charAt(i) == 'a') resultStr += "1010";
    else if (data.charAt(i) == 'b') resultStr += "1011";
    else if (data.charAt(i) == 'c') resultStr += "1100";
    else if (data.charAt(i) == 'd') resultStr += "1101";
    else if (data.charAt(i) == 'e') resultStr += "1110";
    else if (data.charAt(i) == 'f') resultStr += "1111";
  }
  return resultStr;
}

/* the function that converts String representation of binary
    to decimal number in long integer (2's complement)
    @param : String data of binary
    @return : long integer in 2's complement
*/
long binToDec(String data) {
  long result = 0;
  if (data.charAt(0) == '1') {
    for (int i = 0; i < 32; i++) {
      if (data.charAt(i) == '1') data.setCharAt(i, '0');
      else if (data.charAt(i) == '0') data.setCharAt(i, '1');
    }
    for (int i = 31; i >= 0; i--) {
      if (data.charAt(i) == '1') result += integerPow(2, 31 - i);
    }
    result += 1;
    result *= -1;
  }
  else {
    for (int i = 31; i >= 0; i--) {
      if (data.charAt(i) == '1') result += integerPow(2, 31 - i);
    }
  }
  return result;
}

/*
  RepeatingSquare method.
  Decrease the calculation time to O(logN)
  Since std::pow() function has a round-off error,
  please use this function for integer pow
  @param a : long integer to be powered
  @param exponent : power
  @return : a^(exponent)
*/
long integerPow(long a, int exponent) {
  if (exponent == 0) return 1;
  if (exponent % 2 == 0) {
    long t = integerPow(a, exponent/2);
    return t * t;
  }
  return a * integerPow(a, exponent-1);
}

/*
Since std::pow() function has a round-off error,
please use this function for integer pow
@param a : long integer to be powered
@param exponent : power
@return : a^(exponent)
*/
long _integerPow(long a, int exponent) {
  long result = 1;
  for (int i = 0; i < exponent; i++) {
    result *= a;
  }
  return result;
}
