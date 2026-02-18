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

## 📌 Sonuç

- Güç hatları ayrıştırılmıştır.
- Kullanıcı giriş hattı doğrulanmıştır.
- Communication hattı tespit edilmiştir.
- Sinyal genliği ~24–25V olarak ölçülmüştür.

Logic seviyesi 24V olduğu için doğrudan logic analyzer bağlantısı mümkün değildir.
Bir sonraki aşamada uygun level shifting çözümü gereklidir.
