#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define X 5 
#define Y 1
#define Z 1

//Global Degiskenler
int salonkoltuksayisi=X; //bekleme salonu koltuk sayisi
int boskoltuksayisi=0; // bekleme salonu bos koltuk sayisi
int berbersayisi=Y; // berber sayisi
int berberkoltuksayisi=Z; //berber koltuk sayisi
int tirastakimusteri=0;  // o an tiras olan musteri id'sini tutma 
int* koltuk; // musteri-berber arasi degisim ataması icin gerekli
int musterisayisi=0; // musteri sayisi
int salonsandalyeID=0; // bekleme salonunda musterinin oturacağı sandalye id'si


//threadlerin çalışacağı fonksiyonlar
void Berber(void *deger);
void Musteri(void *deger);
void wait();

//SEMAFORLAR
sem_t barber;  //berber semafor
sem_t musteriler;  // musteri semafor
sem_t mutex;	// berber koltuguna erisim kontrolu icin


int main(int argc, char **argv)
{
	
	printf("Müşteri Sayısını giriniz: ");
	scanf("%d",&musterisayisi);

	boskoltuksayisi = salonkoltuksayisi;
	koltuk = (int*) malloc(sizeof(int) * salonkoltuksayisi);

	printf("\nMüşteri Sayısı:  %d",musterisayisi);
	printf("\nBerber Koltuk Sayısı:  %d",berberkoltuksayisi);
	printf("\nBerber Sayısı:  %d",berbersayisi);
	printf("\nBekleme Salonu koltuk Sayısı:  %d\n",salonkoltuksayisi);
	printf("\nSalon boş koltuk Sayısı:  %d\n",boskoltuksayisi);
	int m=0,n=0,i=0;
	m = berberkoltuksayisi;
	n = musterisayisi;
	// threadler tanimlanir
	pthread_t berber[m];        
	pthread_t musteri[n];

	//semaforlar olusturulur
	sem_init(&barber,0,0);
	sem_init(&musteriler,0,0);
	sem_init(&mutex,0,1);

	
	printf("\nDükkan işleme başlıyor..\n\n");

	// berber threadlerini yarat 
	for(i=0;i<berberkoltuksayisi;i++)
	{
	  pthread_create(&berber[i],NULL,(void*)Berber,(void*)&i);
	  sleep(2);
	}
	
	// musteri threadlerini yarat	
	for(i=0;i<musterisayisi;i++)
	{
	  pthread_create(&musteri[i],NULL,(void*)Musteri,(void*)&i);
	  wait(); 
	}  

	// tum musteriler ile ilgilen
	for(i=0;i<musterisayisi;i++)
	{
	  pthread_join(musteri[i],NULL);
	}  
	
	sleep(2);
	
	//semaforlar sonlandırılıyor..
	sem_destroy(&barber);
	sem_destroy(&musteriler);
	sem_destroy(&mutex);
	
	printf("\n\nTüm müşteriler tıraş edildi.Berber uykuya geçti\n\n");
		
	
	return 0;
}

void Berber(void *deger)
{
	int m = *(int*)deger + 1;
	int musteriID, sonrakiMusteri;
	printf("Berber--%d\t dükkana geldi\n",m);
	
	while(1)
	{
	  if(!musteriID) 
	  {
 	    printf("Berber--%d\t uyumaya gitti\n",m);
	  }
  	  sem_wait(&barber);  // berber uykuya gider
	  sem_wait(&mutex); // musterinin berber koltuguna erisimi kilitlenir
	  
	  tirastakimusteri = (++tirastakimusteri) % salonkoltuksayisi;
	  sonrakiMusteri = tirastakimusteri;	 
	  musteriID=koltuk[sonrakiMusteri];
	  koltuk[sonrakiMusteri] = pthread_self();
	  
	  printf("Berber--%d\t %d. müşteri tarafından uyandırıldı.\n\n",m,musteriID);	
	  sleep(1);
	  printf("Berber--%d\t %d. müşteriyi tıraşa başladı.\n\n",m,musteriID);
	  sleep(1);	
	  printf("Berber--%d\t %d. müşteriyi tıraşı bitirdi.\n\n",m,musteriID);	


	  sem_post(&mutex);  //musterinin koltuga erisimi serbest kalır
	  sem_post(&musteriler); //yeni musteriyle ilgili işlem başlar


	  sleep(1);	  
	  printf("Berber--%d\t uyumaya gitti.\n\n",m);				

	}

}

void Musteri(void *deger)
{
	
	int m = *(int *)deger + 1;
	int oturulansandalye,berberID; //bekleme salonunda oturulan sandalye 
	
	sem_wait(&mutex);
	
	printf("Müşteri--%d\t dükkana geldi.\n",m);
	
	if(boskoltuksayisi > 0)
	{
	  boskoltuksayisi--;
	  printf("Müşteri--%d\t bekleme salonunda duruyor.\n",m);		
	
	  salonsandalyeID = (++salonsandalyeID) % salonkoltuksayisi;
	  oturulansandalye = salonsandalyeID;
	  koltuk[oturulansandalye] = m;

  	  sem_post(&mutex);
  	  sem_post(&barber);
	
	  sem_wait(&musteriler);
	  sem_wait(&mutex);	

	  berberID = koltuk[oturulansandalye];
          boskoltuksayisi++;
	  sem_post(&mutex);
	} 
	else
	{
	  sem_post(&mutex);  
	  printf("Müşteri--%d\t bekleme salonunda yer olmadığı için dükkandan ayrıldı.\n",m);
	}
	pthread_exit(0);
}

void wait()
{
	srand((unsigned int)time(NULL));
    	usleep(rand() % (250000 - 50000 + 1) + 50000);

}


