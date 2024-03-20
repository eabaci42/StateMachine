// Önce gerekli enum'ları ve struct'ları tanımlayalım
enum State {
  START,
  Y_RISING,
  Y_MIDDLE,
  Y_FALLING,
  Y_VERTICAL,
  Z_RISING,
  Z_MIDDLE,
  Z_FALLING,
  PRESENTATION
};

enum Button {
  UP_BUTTON,
  DOWN_BUTTON,
  STOP_BUTTON,
  VERTICAL_BUTTON,
  HORIZONTAL_BUTTON,
  NO_BUTTON // Hiçbir butonun basılmadığını gösterir
};

// Her durum için geçerli butonları ve geçişleri tutacak bir yapı
struct StateInfo {
  bool canPressUp;
  bool canPressDown;
  bool canPressStop;
  bool canPressVertical;
  bool canPressHorizontal;
  State nextState[5]; // Her buton için bir sonraki durum
};

// Durum bilgilerini tutacak bir dizi tanımlayalım
StateInfo stateMachine[] = {
  // START durumu - Down, Vertical ve Cap switch'leri basılı olmalı
  {{false, true, false, true, true}, {true, false, false, false, false}, {Y_RISING, START, START, START, START}},
  
  // Y_RISING durumu - Sadece Vertical switch basılı olmalı
  {{false, false, false, true, false}, {false, false, true, false, false}, {Y_RISING, Y_RISING, Y_MIDDLE, Y_RISING, Y_RISING}},
  
  // Y_MIDDLE durumu - Sadece Vertical switch basılı olmalı
  {{false, false, false, true, false}, {true, true, false, false, false}, {Y_RISING, Y_FALLING, Y_MIDDLE, Y_MIDDLE, Y_MIDDLE}},
  
  // Y_FALLING durumu - Sadece Vertical switch basılı olmalı
  {{false, false, false, true, false}, {false, false, true, false, false}, {Y_FALLING, Y_FALLING, Y_MIDDLE, Y_FALLING, Y_FALLING}},
  
  // Y_VERTICAL durumu - Up ve Vertical switch'leri basılı olmalı
  {{true, false, false, true, false}, {true, true, false, false, true}, {Z_RISING, Y_FALLING, Y_VERTICAL, Y_VERTICAL, Z_RISING}},
  
  // Z_RISING durumu - Sadece Up switch basılı olmalı
  {{true, false, false, false, false}, {false, false, true, false, false}, {Z_RISING, Z_RISING, Z_MIDDLE, Z_RISING, Z_RISING}},
  
  // Z_MIDDLE durumu - Sadece Up switch basılı olmalı
  {{true, false, false, false, false}, {true, true, false, true, true}, {Z_RISING, Z_FALLING, Z_MIDDLE, Z_FALLING, Z_RISING}},
  
  // Z_FALLING durumu - Sadece Up switch basılı olmalı
  {{true, false, false, false, false}, {false, false, true, false, false}, {Z_FALLING, Z_FALLING, Z_MIDDLE, Z_FALLING, Z_FALLING}},
  
  // PRESENTATION durumu - Up ve Horizontal switch'leri basılı olmalı
  {{true, false, true, false, false}, {false, true, false, true, false}, {PRESENTATION, Z_FALLING, PRESENTATION, Z_FALLING, PRESENTATION}},
};

struct PinConfiguration {
    int upButtonPin;
    int downButtonPin;
    int stopButtonPin;
    int verticalButtonPin;
    int horizontalButtonPin;

    int upSwitchPin;
    int downSwitchPin;
    int verticalSwitchPin;
    int horizontalSwitchPin;
    int capSwitchPin;
};

// Pin numaralarınızı bu şekilde tanımlayabilirsiniz
PinConfiguration pins = {
    13, // upButtonPin
    12, // downButtonPin
    4, // stopButtonPin
    14, // verticalButtonPin
    27, // horizontalButtonPin

    36, // upSwitchPin
    39, // downSwitchPin
    34, // verticalSwitchPin
    35, // horizontalSwitchPin
    18  // capSwitchPin
};

State currentState = START;

void setup() {
    pinMode(pins.upButtonPin, INPUT);
    pinMode(pins.downButtonPin, INPUT);
    pinMode(pins.stopButtonPin, INPUT);
    pinMode(pins.verticalButtonPin, INPUT);
    pinMode(pins.horizontalButtonPin, INPUT);

    pinMode(pins.upSwitchPin, INPUT_PULLUP);
    pinMode(pins.downSwitchPin, INPUT_PULLUP);
    pinMode(pins.verticalSwitchPin, INPUT_PULLUP);
    pinMode(pins.horizontalSwitchPin, INPUT_PULLUP);
    pinMode(pins.capSwitchPin, INPUT_PULLUP);

    // Seri haberleşme başlat
    Serial.begin(9600);

    // Burada ek başlangıç kodları yer alabilir
}


void loop() {
    // Ana döngü. Buton durumlarını kontrol edip durum makinesini günceller
    Button pressedButton = readButton(pins); // pins yapısını argüman olarak geçirerek buton durumunu oku

    // Mevcut switch durumlarını oku
    SwitchState currentSwitchStates = readSwitchStates(pins); // pins yapısını argüman olarak geçirerek switch durumlarını oku

    // Geçiş mantığı
    if (pressedButton != NO_BUTTON) {
        // Geçerli butona basılıp basılamayacağını kontrol et
        bool canPressButton = false;
        State potentialNextState = stateMachine[currentState].nextState[pressedButton];

        // İlk kontrol: Butona basılabilir mi?
        switch (pressedButton) {
            case UP_BUTTON: canPressButton = stateMachine[currentState].canPressUp; break;
            case DOWN_BUTTON: canPressButton = stateMachine[currentState].canPressDown; break;
            case STOP_BUTTON: canPressButton = stateMachine[currentState].canPressStop; break;
            case VERTICAL_BUTTON: canPressButton = stateMachine[currentState].canPressVertical; break;
            case HORIZONTAL_BUTTON: canPressButton = stateMachine[currentState].canPressHorizontal; break;
            default: break; // Güvenlik için varsayılan bir durum
        }

        // İkinci kontrol: Hedef durumun beklenen switch durumları ile uyumlu mu?
        bool switchStateMatches = checkSwitchStates(pins); // Güncellenmiş checkSwitchStates fonksiyonunu kullan
        
        // Eğer butona basılabilir ve switch durumları uyumluysa, sonraki duruma geçiş yap
        if (canPressButton && switchStateMatches) {
            currentState = potentialNextState;
        }
    }

    // Mevcut durumu işle (örneğin, motorları kontrol et, LED'leri yak/söndür, vs.)
    processCurrentState();
}


// Buton durumunu okuyup ilgili Button enum değerini döndüren fonksiyon
Button readButton(const PinConfiguration& pins) {
    if (digitalRead(pins.upButtonPin) == HIGH) {
        return UP_BUTTON;
    } else if (digitalRead(pins.downButtonPin) == HIGH) {
        return DOWN_BUTTON;
    } else if (digitalRead(pins.stopButtonPin) == HIGH) {
        return STOP_BUTTON;
    } else if (digitalRead(pins.verticalButtonPin) == HIGH) {
        return VERTICAL_BUTTON;
    } else if (digitalRead(pins.horizontalButtonPin) == HIGH) {
        return HORIZONTAL_BUTTON;
    }
    return NO_BUTTON; // Hiçbir buton basılmamış
}

void processCurrentState() {
    switch (currentState) {
        case START:
            // START durumunda yapılacak işlemler
            // Örnek: Bir LED'i yak
            digitalWrite(START_LED_PIN, HIGH);
            // Diğer işlemler...
            break;

        case Y_RISING:
            // Y_RISING durumunda yapılacak işlemler
            // Örnek: Bir motoru çalıştır
            digitalWrite(MOTOR_PIN, HIGH); // Motoru çalıştır
            // Diğer işlemler...
            break;

        case Y_MIDDLE:
            // Y_MIDDLE durumunda yapılacak işlemler
            // İşlemler...
            break;

        case Y_FALLING:
            // Y_FALLING durumunda yapılacak işlemler
            // İşlemler...
            break;

        case Y_VERTICAL:
            // Y_VERTICAL durumunda yapılacak işlemler
            // İşlemler...
            break;

        case Z_RISING:
            // Z_RISING durumunda yapılacak işlemler
            // İşlemler...
            break;

        case Z_MIDDLE:
            // Z_MIDDLE durumunda yapılacak işlemler
            // İşlemler...
            break;

        case Z_FALLING:
            // Z_FALLING durumunda yapılacak işlemler
            // İşlemler...
            break;

        case PRESENTATION:
            // PRESENTATION durumunda yapılacak işlemler
            // Örnek: Tüm LED'leri söndür
            digitalWrite(START_LED_PIN, LOW);
            // Diğer LED ve motor pinleri için de söndürme işlemi yapılabilir
            // Diğer işlemler...
            break;

        default:
            // Tanımlanmamış bir durum varsa, hata ayıklama için bir şeyler yap
            // Örnek: Hata LED'ini yak
            digitalWrite(ERROR_LED_PIN, HIGH);
            break;
    }
}

// Mevcut switch durumlarını okuyan ve currentState'in beklenen switch durumlarıyla uyumlu olup olmadığını kontrol eden fonksiyon

bool checkSwitchStates(const PinConfiguration& pins) {
    // Fiziksel switch durumlarını oku
    bool upSwitchState = digitalRead(pins.upSwitchPin) == LOW; // INPUT_PULLUP modu için LOW basılı anlamına gelir
    bool downSwitchState = digitalRead(pins.downSwitchPin) == LOW;
    bool verticalSwitchState = digitalRead(pins.verticalSwitchPin) == LOW;
    bool horizontalSwitchState = digitalRead(pins.horizontalSwitchPin) == LOW;
    bool capSwitchState = digitalRead(pins.capSwitchPin) == LOW;
    
    // currentState için beklenen switch durumlarını al
    SwitchState expectedSwitchState = stateMachine[currentState].switchState;
    
    // Beklenen switch durumlarıyla karşılaştır
    if ((upSwitchState == expectedSwitchState.upSwitchActive) &&
        (downSwitchState == expectedSwitchState.downSwitchActive) &&
        (verticalSwitchState == expectedSwitchState.verticalSwitchActive) &&
        (horizontalSwitchState == expectedSwitchState.horizontalSwitchActive) &&
        (capSwitchState == expectedSwitchState.capSwitchActive)) {
        return true; // Mevcut switch durumları beklenen durumlarla uyumlu
    } else {
        return false; // Beklenen durumlarla uyumsuzluk var
    }
}

SwitchState readSwitchStates(const PinConfiguration& pins) {
    SwitchState state;
    state.upSwitchActive = digitalRead(pins.upSwitchPin) == LOW; // INPUT_PULLUP modu için LOW basılı anlamına gelir
    state.downSwitchActive = digitalRead(pins.downSwitchPin) == LOW;
    state.verticalSwitchActive = digitalRead(pins.verticalSwitchPin) == LOW;
    state.horizontalSwitchActive = digitalRead(pins.horizontalSwitchPin) == LOW;
    state.capSwitchActive = digitalRead(pins.capSwitchPin) == LOW;
    return state;
}


