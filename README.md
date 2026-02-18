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

## 🔄 Master/Slave Tespiti

Bit frameleri doğru şekilde yakalandıktan sonra, byte-level analiz ve
paket çözümlemesi için hangi tarafın master (sorgulayan) ve hangi tarafın
slave (cevaplayan) olduğunu belirlemek gerekiyordu.

Protokol tek hatlı olduğundan:

- Bir taraf sürekli dinlemede kalıyor  
- Diğer taraf sorgulama (polling) yapıyor  

Hangi tarafın master olduğunu anlamak için:

1. Haberleşme hattı geçici olarak kesildi  
2. Süpürge çalıştırıldı  
3. İlk konuşma denemesi her iki taraftan ayrı ayrı dinlendi  

### 📌 Sonuç

- **Master / Polling yapan taraf:** Süpürge  
- **Slave / Cevap veren taraf:** Batarya

Bu tespit, veri setinin doğru şekilde analiz edilmesini ve
sonraki aşama olan **alan tespiti / field identification** için sağlam bir temel sağlar.

## 📦 Paket Başlangıç ve Bitiş Condition’ları

Logic analyzer ile alınan byte akışı incelendiğinde, tekrarlı bir pattern gözlemlenmiştir:

- **0xFB** → Paket başlangıcı  
- **0xFC** → Paket sonu  

Dikkat çekici nokta:  
- 0xFB’den sonra 12 byte ileride 0xFC  
- 0xFC’den sonra 8 veya 9 byte sonra tekrar 0xFB  

Bu patternlerin paket başı ve paket sonu condition’ları olduğu varsayılmıştır.

---

## 📝 Excel Tablosuna Aktarım

Bu paket başı/sonu condition’larına göre, örnek bir konuşma akışı:

- Her 0xFB…0xFC paketi → Süpürgeden bataryaya  
- Her 0xFC…0xFB paketi → Bataryadan süpürgeye

Bu paketleri **satırlara ayırarak Excel tablosuna** döktüm.  
Henüz her byte’ın anlamını bilmiyor olsak da, tekrarlı alanlar gözlemlenebiliyordu.

- **Sarı arkaplanlı sütunlar** → Süpürgeden bataryaya giden paketler  
- **Mavi arkaplanlı sütunlar** → Bataryadan süpürgeye giden paketler

### 📊 Örnek Excel Görseli

<img src="ASSETS/example_packet_table.png" alt="Excel Paket Tablosu Örneği" width="800">

## 🔗 Paket İçi Korelasyon ve İlk Byte Analizi

Excel tablosunu dikkatle incelediğimizde bazı anlamlı korelasyonlar ortaya çıkmıştır:

Örnek bir süpürge → batarya paketi:

| Byte # |0    | 1   | 2  | 3  | 4 | 5| 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 |
|--------|-----|-----|----|---|---|---|---|---|---|----|----|----|----|----|
| Paket  | FB  | 41  | 45 | 0B| 00| 00| 00| 00| 00| 09 | 00 | 9A | 00 | FC |

Karşılık gelen batarya → süpürge paketi:

| Byte # |0    | 1   | 2   | 3  | 4  | 5  | 6  | 7  | 8  | 9  | 10 | 11 | 12 | 13 |
|--------|-----|-----|----|----|----|----|----|----|----|----|----|----|----|----|
| Paket  | FC  | 45  | 41 | 44 | 64 | 64 | 00 | 92 | 01 | FB |    |    |    |    |

### 📌 İlk Korelasyon Çıkarımı

- **1. byte (0x41)** → Kaynak ID (source ID)  
- **2. byte (0x45)** → Hedef ID (destination ID)  

Önce süpürgeden gelen pakette, ardından bataryadan gelen pakette bu değerlerin karşılıklı olarak eşleştiği gözlemlenmiştir.  

Bu korelasyon, **master/slave ve adresleme mekanizması** hakkında ilk ipuçlarını vermektedir.

### ✅ Checksum / Veri Bütünlüğü Doğrulama

İstisnasız her pakette geçerli olan **source/target ID korelasyonu** sayesinde
çözümlenmesi gereken **son 2 byte alanı** daha anlamlı hale gelmiştir.  

Çoğu seri haberleşme protokolünde olduğu gibi, G11 batarya protokolünde de
**checksum veya CRC benzeri bir veri bütünlüğü doğrulama alanı** vardır.

#### 🔹 İlk Varsayım

- Her paketin **son 2 byte’ı** checksum alanı olarak kabul edildi  
- Paket başı ve paket sonu condition’ları bu hesaba dahil edilmedi  
- Paket boyutu değişken olsa bile bu varsayım uygulanabilir

#### 🔹 Doğrulama

- Seçilen örnek paketler üzerinde **checksum hesaplaması** yapıldı  
- Hesaplama yöntemi:  checksum = SUM(paket başı/sonu durumları ve checksum alanı hariç tüm byte'lar)
- Tüm test edilen paketlerde hesaplanan checksum ile paket içindeki son 2 byte **tam olarak eşleşti**  

#### 📊 Örnek Paket ve Checksum

| Paket (süpürge → batarya) | Byte 0 | Byte 1 | … | Byte n-2(Checksum_L) | Byte n-1(Checksum_H) | Byte n |
|----------------------------|--------|--------|---|----------|----------|--------|
| Örnek 1                   | 0xFB   | 0x41   | … | 0x9A     | 0x00     | 0xFC   |

Hesaplanan Checksum        0x41+0x45+0x0B+0x09=0x009A

> Son 2 byte paket içindeki checksum ile tam olarak eşleşmektedir.

### 📊 Tüm Paketler Üzerinde Checksum Doğrulama

Tek paket üzerinden yapılan denemeler yeterli kanıt sağlamadığından,
checksum doğrulamasını **tüm veri setine** uyguladım:

- Excel tablosunda, paket başı ve paket sonu condition’ları dikkate alınmadan
  her paketin son 2 byte’ının checksum olduğu varsayıldı  
- Paket içerisindeki **checksum alanı** ile **hesaplanan checksum** karşılaştırıldı  
- Bu karşılaştırmayı otomatik yapan **formüllü bir sütun** oluşturuldu

#### 🔹 Sonuç

- Toplam ~6500 paket üzerinde doğrulama yapıldı  
- Tek bir paket bile formül doğrulamasını ihlal etmedi  

> Bu sayede, checksum alanı kesin olarak doğrulanmış oldu.

#### 🧮 Checksum Hesaplama

Aşağıdaki formül, paket tipine göre hangi byte’ların checksum’a dahil edileceğini belirler ve checksum'u hesaplar:

```
=EĞER([@1]=41;
    DEC2HEX(TOPLA(HEX2DEC([@1]);HEX2DEC([@2]);HEX2DEC([@3]);HEX2DEC([@4]);HEX2DEC([@5]);HEX2DEC([@6]);HEX2DEC([@7]);HEX2DEC([@8]);HEX2DEC([@9]);HEX2DEC([@10]));4);
EĞER([@1]=45;
    DEC2HEX(TOPLA(HEX2DEC([@1]);HEX2DEC([@2]);HEX2DEC([@3]);HEX2DEC([@4]);HEX2DEC([@5]);HEX2DEC([@6]));4);
EĞER([@1]=42;
    DEC2HEX(TOPLA(HEX2DEC([@1]);HEX2DEC([@2]);HEX2DEC([@3]);HEX2DEC([@4]);HEX2DEC([@5]);HEX2DEC([@6]);HEX2DEC([@7]));4)
)))
```
Checksum OK? Sutünündaki bu formülde hesaplanan checksum ile checksum alanındaki değerin eşit olup olmama duruma göre OK veya ERRROR döndürür

```
=EĞER([@Checksum]=
EĞER([@1]=41;
  DEC2HEX(BİTVEYA(BİTSOLAKAYDIR(HEX2DEC([@12]);8);HEX2DEC([@11]));4);
EĞER([@1]=45;
  DEC2HEX(BİTVEYA(BİTSOLAKAYDIR(HEX2DEC([@8]);8);HEX2DEC([@7]));4);
EĞER([@1]=42;
  DEC2HEX(BİTVEYA(BİTSOLAKAYDIR(HEX2DEC([@9]);8);HEX2DEC([@8]));4);0
)));"OK";"ERROR")
```

#### 📷 Excel Checksum Alanı Doğrulama Görseli

<img src="ASSETS/excel_checksum_validation.png" alt="Excel Checksum Validation" width="800">

