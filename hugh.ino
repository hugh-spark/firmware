int last_red = -1;
int last_green = -1;
int last_blue = -1;
int last_brightness = -1;
int last_timestamp = 0;
char msg[255] = "";
int activity_level = 0;
int last_local_activity = 0;

// This routine runs only once upon reset
void setup() {
    // Serial sometimes takes a bit to get ready
    Serial.begin(9600);
    delay(1000);
    Serial.print("Hello ");
    
    // Take control of the Spark core's RGB LED
    RGB.control(true);
    controlBulb(32, 96, 128, 32);

    Spark.function("command", parseCommand);
    Spark.function("activity_level", simulateLocalActivity);
    Spark.variable("last_red", &last_red, INT);
    Spark.variable("last_green", &last_green, INT);
    Spark.variable("last_blue", &last_blue, INT);
    Spark.variable("last_brightness", &last_brightness, INT);
    Spark.variable("last_timestamp", &last_timestamp, INT);
    Spark.variable("activity_level", &activity_level, INT);
    Spark.variable("msg", msg, STRING);
    
    Serial.println("World");
}

// This routine loops forever 
void loop() {
    if (activity_level > 0) {
        int activity_level = constrain(100 - activity_level, 1, 100);
        int now = Time.now();
        if ((now - last_local_activity) >= activity_level) {
            // Simulate somebody changing our settings locally
            controlBulb(rand() % 255, rand() % 255, rand() % 255, rand() % 255);
            last_local_activity = now;
        }
    }
    // Don't thrash too hard checking
    delay(1000);
}

bool controlBulb(int red, int green, int blue, int brightness) {
    return controlBulb(red, green, blue, brightness, Time.now());
}

bool controlBulb(int red, int green, int blue, int brightness, int timestamp) {
    if (timestamp >= last_timestamp) {
        RGB.brightness(brightness);
        RGB.color(red, green, blue);
        
        last_red = red;
        last_green = green;
        last_blue = blue;
        last_brightness = brightness;
        last_timestamp = timestamp;
        
        Spark.publish("state", String(last_red) + "," + String(last_green) + "," + String(last_blue) + "," + String(last_brightness) + "," + String(last_timestamp));
        
        return true;
    } else {
        return false;
    }
}

bool parseNextValue(String input, int *offset, int *value) {
    int next_offset = input.indexOf(',', *offset);
    String candidate_substring = (next_offset >= 0) ? input.substring(*offset, next_offset) : input.substring(*offset);
    int candidate_integer = candidate_substring.toInt();
    *offset = next_offset + 1;
    if ((String(candidate_integer).compareTo(candidate_substring) == 0) && (0 <= candidate_integer) && (candidate_integer <= 255)) {
        *value = candidate_integer;
        return true;
    }
    return false;
}

// POST https://api.spark.io/v1/devices/<device name>/command
// {"params": "99,127,0,255"}"
// R,G,B,brightness all [0, 255]
int parseCommand(String command) {
    int offset = 0;
    int red, green, blue, brightness = -1;
    
    if (parseNextValue(command, &offset, &red)
        && parseNextValue(command, &offset, &green)
        && parseNextValue(command, &offset, &blue)
        && parseNextValue(command, &offset, &brightness)
        && controlBulb(red, green, blue, brightness)
    ) {
        return 0;
    } else {
        return 1;
    }
}

// POST https://api.spark.io/v1/devices/<device name>/activity_level
// {"params": "100"}"
// A value of 0 will turn off local activity
// Otherwise, higher numbers will result in more frequent activity, to a
// maximum level of 100, which should have an activity every second.
int simulateLocalActivity(String command) {
    activity_level = command.toInt();
    String debug = "Activity level set to " + String(activity_level);
    debug.toCharArray(msg, 250);
    return 0;
}
