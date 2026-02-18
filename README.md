# 🔋 G11 Battery Protocol Emulation

## 📌 Projenin Amacı

Bu proje, akıllı bir batarya ile cihaz arasındaki haberleşme protokolünün
reverse engineering yöntemiyle analiz edilmesi ve gömülü sistem üzerinde
yeniden modellenmesini konu almaktadır.

Amaç:

- Kapalı bir sistemin communication davranışını analiz etmek
- Packet yapısını çözümlemek
- Doğrulama (validation) mekanizmalarını anlamak
- Elde edilen verilerle protocol-level emulation geliştirmek

Bu çalışma, planlı eskitme tartışmalarına teknik bir perspektiften yaklaşır.
Bir sistemi gerçekten anlamanın yolu, onu yeniden inşa edebilmekten geçer.

---

## 🔬 Çalışma Süreci (Özet)

Bu proje aşağıdaki deterministik mühendislik adımlarını takip etmiştir:

1. Arayüz Karakterizasyonu
2. Veri Yakalama
3. Alan Tespiti
4. Algoritma Doğrulama
5. Protokol Emülasyonu
6. Donanım Arayüz Tasarımı
---

# 1️⃣ Arayüz Karakterizasyonu

Batarya paketi ve süpürge gövdesi açılmadan analiz yapılması hedeflenmiştir.
Bu nedenle konnektör pin fonksiyonları dolaylı ve non-invasive yöntemlerle
tespit edilmiştir.

## 📷 Referans PCB Görselleri

Model yeni olduğu için teardown materyali sınırlıdır.

Yapılan araştırmalar sonucunda:

- Batarya PCB görüntüsüne bir video incelemesinde,
- Süpürge tarafındaki PCB görüntüsüne online yedek parça platformunda

ulaşılmıştır.

> Batarya PCB’sinde pin fonksiyonlarına dair işaretleme bulunmamaktadır.  
> Süpürge tarafındaki PCB üzerinde konnektör pin isimleri etiketlenmiştir.

📌 Referans Görseller:

### 🔋 Batarya PCB fotoğrafı

![Battery PCB Reference](ASSETS/battery_pcb_reference.jpg)

### 🧹 Süpürge PCB fotoğrafı

![Vacuum PCB Reference](ASSETS/vacuum_pcb_reference.png)

---

## 🧩 Konnektör Pin Dizilimi

10 pinli konnektör üzerinde yer alan pin isimleri:

P- | P- | P- | UI- | S | KEY | UI+ | P+ | P+ | P+

Ancak ilgili PCB'nin birebir aynı revizyona ait olduğu kesin olmadığı için
tüm pin fonksiyonları elektronik olarak doğrulanmıştır.

---

## 🔬 Elektriksel Doğrulama

Batarya ve süpürge konnektörleri jumper kablolar ile açığa alınmış,
cihaz çalışırken ölçümler gerçekleştirilmiştir.

### ⚡ Güç Hatları

- P- / P+ → Sürekli 24–25V DC  
  → Ana güç hattı olarak doğrulanmıştır.

### 🖥 UI Hatları

- UI- / UI+ → Yalnızca ekran aktifken 24–25V  
  → Ekran besleme hattı olarak değerlendirilmiştir.

### 🎯 KEY Hattı

- Tetik basılı → 24–25V  
- Tetik bırakıldığında → 0V  

→ Kullanıcı giriş hattı doğrulanmıştır.

### 📡 S Hattı

- Cihaz çalışırken → 24–25V genliğinde periyodik kare dalgalar  
- Cihaz kapalıyken → 0V sabit  

Bu davranış hattın communication line olduğunu güçlü şekilde göstermektedir.

---

### 🏷 Pin Mapping Etiketleme

Daha sonraki analiz çalışmalarında bağlantı hatalarını önlemek ve ölçüm
tekrar edilebilirliğini sağlamak amacıyla tespit edilen pin mapping,
hem batarya hem de süpürge konnektörü üzerinde fiziksel olarak etiketlenmiştir.

Bu sayede:

- Ölçüm noktaları standardize edilmiştir.
- Yanlış bağlantı riski minimize edilmiştir.
- Veri yakalama aşamasında referans karışıklığı önlenmiştir.

### 🔖 Etiketlenmiş Konnektör Görselleri

<img src="ASSETS/battery_connector_labeled.jpg" alt="Battery Connector Labeling" width="500"> <img src="ASSETS/vacuum_connector_labeled.jpg" alt="Vacuum Connector Labeling" width="400">

---

## 📌 Sonuç

- Güç hatları ayrıştırılmıştır.
- Kullanıcı giriş hattı doğrulanmıştır.
- Communication hattı tespit edilmiştir.
- Sinyal genliği ~24–25V olarak ölçülmüştür.

Logic seviyesi 24V olduğu için doğrudan logic analyzer bağlantısı mümkün değildir.
Bir sonraki aşamada uygun level shifting çözümü gereklidir.

# 2️⃣ Veri Yakalama ve Protokol Keşfi Denemeleri

Batarya konnektörü ve level shifter devresi kurulduktan sonra
S hattından jumper alınıp logic analyzer girişine bağlanmıştır.  
Hattın güvenli şekilde dinlenmesi sağlanmıştır.

---

## 🔍 İlk Analiz: 1-Wire Hipotezi

Hattın tek hatlı olması nedeniyle ilk olarak **1-Wire protokolü** varsayılmıştır.  

- Bazı anlamlı byte'lar gözlemlense de  
- Çok sayıda framing hatası ve korelasyonsuz byte dizileri mevcuttu  

Capture ve ekran görüntüleri:

[Capture Dosyası (Session 0.sal)](DOCUMENT/Session%200.sal)

<img src="ASSETS/Logic_Sesion0.png" alt="Logic Analyzer Capture" width="400">

Bu gözlemler, hattın **standart 1-Wire protokolü olmadığı** ihtimalini güçlendirdi.

---

## ⚡ İkinci Analiz: Half-Duplex Single-Wire UART Hipotezi

Daha sonra hattın **half-duplex single-wire UART** olabileceği üzerine yoğunlaşıldı.  

- En yaygın standart baudrate değerlerinde sinyal analiz edildi  
- Hala çok sayıda framing hatası ve korelasyon gözlemlendi  

---

## ✅ Çözüm: Invert ve Doğru Parametreler

Son bir deneme olarak sinyal invert edilerek analiz edildi ve:

- **8N1 standardı**  
- **9600 baudrate**  
- **Inverted signal**

parametreleri ile **frameler tam olarak oturdu**.  


Capture ve ekran görüntüleri:

[Capture Dosyası (.sal)](DOCUMENT/Session%201.sal)  

<img src="ASSETS/Logic_Sesion1.png" alt="Logic Analyzer Capture" width="400"> <img src="ASSETS/valid_uart_configuration.png" alt="Protocol Analyzer Konfigürasyonu" width="400">

Byte dizileri artık **istikrarlı ve tekrar eden korelasyonlar** göstermeye başladı.  
Bu sayede protokolün fiziksel ve temel veri yapısı net bir şekilde ortaya çıktı.

📌 Bu aşamadan sonra **alan tespiti ve byte-level analiz** adımına geçilmiştir.
