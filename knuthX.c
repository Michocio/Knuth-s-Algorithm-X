/*
*
*	Program zaliczeniowy nr. 3
*	Wstęp do Programowania
*	Potok Imperatywny 2015
*
*	Problem dokładnego pokrycia
*
*	@author Michał Jaroń
*	<mj348711.students.mimuw.edu.pl>
*	nr. indeksu: 348711
* 	@copyright Uniwerstet Warszawski
*	@date: 05.01.2016
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>


/*	Opis rozwiązania:
*	Moje rozwiązanie wzoruje się
*	na podejściu przedstawionym
*	przez samego Donald E. Knuth,
*	który technike, pozwalającą efektywnie
* 	znajdować dokładne pokrycie nazwał
*	"Dancing Links". Zgodnie z formatem danych
*	na wejściu. Reprezentuję dane w 
* 	postaci macierzy. Jako rzecz macierz ta
*	na ogół jest macierzą rzadką, to nie
*	w celu minimalizowania użycia pamięci
*	przechowuje kolejne wiersze macierzy
*	(reprezentujące elementy zaliczającę sie
*	do kolejnych zbiorów) w postaci list.
*	Pozwala to zmniejszyć zużycie pamięci do minimum,
*	jednak co najważniejsze pozwala efektywnie
*	rozwiązać postawiony problem. 
*	W kodzie wielokrotnie posługuję się
*	określeniem "element" - mam na myśli
* 	znaki pojawiające się na wejściu,
*	z których tworzę "większe" byty, ponieważ
*	de facto każdy znak nie będący spacją
*	to "1" określająca, że do danego zbioru 
* 	(wiersza macierzy) należy dana wartość 
* 	(kolumna macierzy).
* 
*	Rozwiązując zadanie wzorowałem się na pracy:
* 
* 			http://arxiv.org/pdf/cs/0011047v1.pdf */





/*	Struktura pozwalająca na
*	przechowywanie wczytanych danych
*	i powiązanych z nimi atrybutów*/
typedef struct obiekt
{
	int war; /*	Wartość danego obiektu */
	int poz; /* Który, z kolei niepusty znak 
				w danym wierszu */  
	
	/* Wspołrzędne elementu w "macierzy" */
	int x, y; /* x - numer kolumny, 
				y - numer wiersza */
	
	/*	Wskaźniki pokazujące na sąsiadujące elementy */
	struct obiekt *lewo, *prawo;
	struct obiekt *gora, *dol;
}obiekt;



void algorytm(int szer_max, obiekt **kolumna_start[], 
			obiekt **kolumna_koniec[]);	


/*	Procedura dodająca nowo wczytany element 
*	do tablicy wszystkich elementów.
*	[in] co - wartość wczytanego znaku
*	[in] wsp_x, wsp_y - położenie elementu w macierzy,
* 			obliczone w trakcie wczytywania danych 
* 			w procedurze wczytaj_dane.
*	[in,out] elementy - tablica przechowująca 
* 				wszystkie elementy,
* 				to do niej dodawany nowo wczytany znak.
*	[in,out] rozmiar - licznik liczby 
* 						wczytanych elementów */
void dodaj(int co, int wsp_x ,int wsp_y, int poz,
			obiekt **elementy[], int *rozmiar)
{
	
	/*	Rezerwuj pamięć dla nowego obiektu */
	obiekt *nowy = (obiekt*) malloc( sizeof (obiekt) );
	
	/*Zwiększ liczbę dodanych elementów */
	*rozmiar+=1;
	
	/* Inicjalizuj wartości nowego obiektu */
	nowy->war = co;
	nowy->x = wsp_x;
	nowy->y = wsp_y;
	nowy->poz = poz;
	
	/*	Powiększ tablicę przechowującą elementy */
	*elementy = realloc ( *elementy, 
						sizeof (obiekt*) * *rozmiar);
	/*	Dodaj nowy element do tablicy */ 
	(*elementy)[*rozmiar-1] = nowy;	
}

/*	Procedura inicjalizuje tablicę przechowującą 
* 	wczytane elementy. Na początku tablicy wstawia
*	"zaślepke" - element o wartości -1 
*	[in,out] elementy - tablica przechowująca 
* 				wszystkie elementy,
* 				to do niej dodawany nowo wczytany znak.
*	[in,out] rozmiar - licznik liczby 
* 						wczytanych elementów. */
void init_elementy(obiekt **elementy[], int *rozmiar)
{
	/*	Rezerwuj pamięć dla atrapy */
	obiekt *nowy = (obiekt*) malloc( sizeof (obiekt) );
	
	/* Inicjalizuj wartości atrapy */
	nowy->war = -1;
	nowy->poz = 0;
	nowy->x = 0;
	nowy->y = 0;
	
	/*	"Stwórz" tablicę przechowującą elementy */
	*elementy=  malloc ( sizeof (obiekt*) ); 
	
	/*	Dodaj atrape do tablicy */ 
	*elementy[0]=nowy;
	/* Początkowy rozmiar tablicy */
	*rozmiar+=1;
}


/*	Procedura tworząca dowiązaniową strukutre danych,
*	odwzorowującą macierz rzadką. Procedura przetwarza
*	zapisane w elementach współrzędne, ustalone w czasie
*	wczytywania danych i na podstawie tych współrzędnych
*	tworzy odpowiednie dowiązania.
*	Czyni nawigacje po elementach łatwą, za pomocą
*	list dwukierunkowych, w każdej z orientacji
* 	(poziomo, pionowa).
*	[in,out] elementy - wczytane elementy
*	[in] szerokosc[] - tablica trzymająca szerokości
* 						kolejnych wierszy.
*  	[in] wysokosc[] - tablica trzymająca wysokosci
* 						kolejnych kolumn.
*	[in] szer_max - szerokość najdłuższego wiersza.
*	[in] wys_max - wysokość najwyższej kolumny 
* 					(liczba węzłów).
* 	[in,out] wiersz_start []- tablica przechowująca dowiązania
*					do początków kolejnych wierszy.
*	[in,out] wiersz_koniec []- tablica przechowująca dowiązania
*					do końców kolejnych wierszy.
*	[in,out] kolumna_start []- tablica przechowująca dowiązania
*					do początków kolejnych kolumn.
*	[in,out] wiersz_koniec []- tablica przechowująca dowiązania
*					do końców kolejnych kolumn. */
void tworz_strukture (obiekt **elementy[], int *szerokosc, 
						int szer_max, int wys_max ,
						obiekt **wiersz_start[], 
						obiekt **wiersz_koniec[], 
						obiekt **kolumna_start[], 
						obiekt **kolumna_koniec[])
{
	/*	Pomocnicze iterator, używane w poniższych
	 *	pętlach. */
	int i=0;
	int wsp_y=0, wsp_x=0;
	int index=1;
	
	/*	Alokacja pamięci dla tablic */
	*wiersz_start= malloc ( sizeof(obiekt*) * wys_max);
	*wiersz_koniec= malloc ( sizeof(obiekt*) * wys_max);
	
	*kolumna_start = malloc ( sizeof(obiekt*) * szer_max);
	*kolumna_koniec= malloc ( sizeof(obiekt*) * szer_max);

	/*	Iniciuj tablice kolumn */
	for(i=0;i<szer_max;i++)
	{
		(*kolumna_start)[i]=NULL;
		(*kolumna_koniec)[i]=NULL;
	}

	/*	Przejrzyj wszystkie elementy */
	for(wsp_y=0; wsp_y < wys_max; wsp_y++)
	{/*	Wierszami */
		for(wsp_x=0; wsp_x < szerokosc[wsp_y]; wsp_x++)
		{/*	Kolumnami */
			/*	Ktory wiersz */
			int ktora=(*elementy)[index]->x;
			
			/*	Początek nowego wiersza */
			if(wsp_x == 0)
				(*wiersz_start)[wsp_y]=(*elementy)[index];
			else
			{/*	Kolejny element dopisany do wiersza */
				(*elementy)[index]->lewo=(*wiersz_koniec)[wsp_y];
				(*elementy)[index-1]->prawo=(*elementy)[index];
			}

			/*	Nowa kolumna */
			if((*kolumna_start)[ktora]==NULL)
			{
				(*kolumna_start)
							[ktora]=(*elementy)[index];		
			}
			else
			{/*	Dowiąż do istniejącej kolumny */
				(*elementy)[index]->gora=(*kolumna_koniec)[ktora];
				(*kolumna_koniec)[ktora]->dol=
								(*elementy)[index];
			}	
				
			/*	Zadbaj jeszcze o wskaźniki do końców */	
			(*wiersz_koniec)[wsp_y]=(*elementy)[index];
			(*kolumna_koniec)[ktora]=(*elementy)[index];

			index++;
		}
		
		/*	Uczyń listę wierszy cykliczna, poprzez zapętlenie
		 * 	wskaźników */
		(*wiersz_start)[wsp_y]->lewo= (*wiersz_koniec)[wsp_y];
		(*wiersz_koniec)[wsp_y]->prawo= (*wiersz_start)[wsp_y];
	}			
	
	/*	Uczyń listę kolumn cykliczna */
	for(i=0;i<szer_max;i++)
	{
		(*kolumna_start)[i]->gora= (*kolumna_koniec)[i];
		(*kolumna_koniec)[i]->dol= (*kolumna_start)[i];
	}		

}

/*	Procedura wczytująca i wstępnie "obrabiającą"
*	dane z wejścia. Czyta znaki z wejścia aż do
*	końca danych (EOF). Równocześnie zapisuje wczytane
*	znaki poprzez procedure dodaj. Podczas wczytywania
*	zapamiętywane są atrybutu związane z kolejnością
*	napływających elementów, które pozwalają
*	na określenie dokłądnego położenia elementy
*	w tworzonej potem dowiązaniowej strukturze danych
*	[in,out] elementy - tablica gdzie przechowywane
*					spływające elementy
*	[in,out] rozmiar - ile wczytanych znaków różnych od
* 				spacji (rozmiar tablicy elementy)
*	[in,out] szerokosc[] - tablica trzymająca szerokości
* 						kolejnych wierszy.
*  	[in,out] wysokosc[] - tablica trzymająca wysokosci
* 						kolejnych kolumn.
* 	[in,out] szer_max - szerokość najdłuższego wiersza.
*	[in,out] wys_max - wysokość najwyższej kolumny 
* 					(liczba węzłów). */
void wczytaj_dane(obiekt **elementy[], int *rozmiar,
				int **szerokosc, int **wysokosc, 
				int *szer_max, int *wys_max )
{	
	/*	Flaga sygnalizuje czy wczytano cokolwiek - 
	 *	czy wiersz nie jest pusty - zabezpieczenie 
	 * 	przed brakiem danych wejściu */
	int wsp_y=0; bool flaga=false;

	
	bool stop=false;
	while(stop != true)
	{
		/*	C - wczytywany znak */
		int c;
		/*	Nowy wiersz - zeruj współrzędne */
		int poz = 0;	int wsp_x = 0;
		
		/*	Wczytuj wiersz, aż do znaku nowej linii 
		 * lub końca danych */
		while( (c = getchar() ) != EOF && c != '\n')
		{	
			if(c!=' ')/* Znak różny od spacji */
			{
				flaga = true;/*	Niepusty wiersz */
				
				/*	Dodaj wczytany znak do tablicy */
				dodaj(c, wsp_x, wsp_y, poz, 
						elementy, rozmiar);
				poz++;/* Który z kolei znak niebedący spacją,
						w danej linii */
			}
			wsp_x++;/*	Kolejna kolumna */
			
			if( wsp_x > *szer_max )
			{/* Nowo wczytany znak rozszerze szerokość macierzy */
				*szer_max = wsp_x;
				/*	Powiększ tablice */ 
				(*wysokosc) = realloc ( (*wysokosc), 
								sizeof(int) * wsp_x);
				
				/*	Pamiętaj wyskość danej kolumny */
				if(c!=' ')
					(*wysokosc)[wsp_x-1]=1;
				else
					(*wysokosc)[wsp_x-1]=0;	
			}
			else
			{/* Nowo wczytany znak NIE 
				rozszerze szerokość macierzy */
				if(c!=' ')
					(*wysokosc)[wsp_x-1]+=1;
			}	
		}

		if(flaga)
		{/*	Wczytano niepusty wiersz */
			if(wsp_x > *szer_max)
				*szer_max=wsp_x;
				
			/*	Powiększ numer wiersza, 
				do którego piszemy */	
			if(flaga)
				wsp_y++;
				
			flaga=false;
			(*szerokosc) = realloc ( (*szerokosc), 
								sizeof(int) * wsp_y );
			/*	Pamiętaj szerokość danego wiersza */					
			(*szerokosc)[wsp_y-1] = poz ;
		}	
		
		flaga=false;
		if(c==EOF)
		{/*	Koniec danych */
			
			/*	Posprzątaj na koniec */
			*wys_max= wsp_y;
			stop=true;
		}
			
	}
}


/*	Garbagge collector. Dealokuje pamięć
*	zaalokowańa na potrzeby rozwiązania */	
void sprzataj( obiekt **elementy[], int rozmiar,
						obiekt **wiersz_start[], 
						obiekt **wiersz_koniec[], 
						obiekt **kolumna_start[], 
						obiekt **kolumna_koniec[],
						int **szerokosc, int **wysokosc)
{
	int i;
	for(i=0;i<rozmiar;i++)
		free((*elementy)[i]);
		
	free((*elementy));	
	
	free((*wiersz_start));
	free((*wiersz_koniec));
		
	free((*kolumna_start));
	free((*kolumna_koniec));
	
	free((*szerokosc));	
	free((*wysokosc));

}

	
	
/**************************************
***********************
**********
******
***/ 	
int main()
{
	/*	Tablica przechowująca wczytane znaki,
	 * 	oraz jej rozmiar - liczba wcczytanych znaków,
	 * 	różnych od spacji */
	obiekt **elementy; int rozmiar=0;
	
	/*	Tablice kolumn zawierające odwołanie
	 *	do elementów w macierz. Nazwy dość sugestywne.
	 * 	Ułatwiają nawigację po macierzy */
	obiekt **wiersz_start, **wiersz_koniec;
	obiekt **kolumna_start, **kolumna_koniec;

	/*	Tablica rozmiarów macierz */
	int *szerokosc=NULL;
	int *wysokosc=NULL;
	
	/*	Boki macierzy*/
	int szer_max=0, wys_max=0;

	
	/*	Wstępna inicjalizacja tablicy 
	 * 	przechowującej elementy*/
	init_elementy(&elementy, &rozmiar);
	
	/*	Czytaj z wejście */
	wczytaj_dane( &elementy, &rozmiar, 
					&szerokosc, &wysokosc,
					&szer_max, &wys_max);
				

	/*	Przetwórz wejście na macierz */
	tworz_strukture(&elementy, szerokosc, szer_max, wys_max,
					&wiersz_start, &wiersz_koniec, 
					&kolumna_start, &kolumna_koniec);
				
	/*	Szukaj odpowiedzi */
	algorytm(szer_max, &kolumna_start, &kolumna_koniec);
		

	sprzataj(&elementy, rozmiar,&wiersz_start, &wiersz_koniec, 
						&kolumna_start, &kolumna_koniec,
						&szerokosc, &wysokosc);
	
	return 0;
}
/****
*************
******************
************************
***************************
*******************************/

							

/*	Deklaracja funkcji pokrycie, tak, żeby
*	możliwe było jej użycie przed definicją,
*	w procedurze algorytm. Sam opis działanie funkcji
*	przy komentarzu, proszę przesunąć w dół */
int pokrycie(obiekt **kolumny, char *odp,
				obiekt **tab_kol[], obiekt **kolumna_start[], 
				int k, obiekt **O[], int num[]);



/*	Procedura stwarzająca "środowisko" do 
*	wywołania faktycznego algorytmu znajdowanie
*	dokładnego pokrycia.
*	Procedura tworzy dodatkowe struktury danych,
*	potrzebne w działaniu funkcji pokrycie,
*	oraz urchumaia rekurencje, z parametrem k=0.
*	Kiedy funkcja pokrycie wróci z rekurencji,
*	procedura algorytm sprząta dodatkowe struktury,
*	utworzone na porzeby funkcji pokrycie
*	[in] szer_max - szerokość najdłuższego wiersza.
*	[in,out] kolumna_start []- tablica przechowująca 
* 				dowiązania do początków kolejnych kolumn.
*	[in,out] kolumna_koniec []- tablica przechowująca 
* 				dowiązania do końców kolejnych kolumn. */
void algorytm(int szer_max, obiekt **kolumna_start[], 
			obiekt **kolumna_koniec[])	
{	
	/*	Lokalne zmienne */
	int i; /* Służy, we wszystkich w pętlach FOR 
			* w procedurze */
			
	/*	Przy znajdowaniu dokładnego pokrycie,
	 * w tablicy num zapisywane kolejne wiersze -
	 * kandydaci na zbiór należący do dokładnego pokrycia */		
	int *num= malloc ( sizeof (int) * (szer_max+1));
	
	/*	Przy znajdowaniu dokładnego pokrycie,
	 * w tablicy odpowiedz zapisywane kolejne znaki -
	 * kandydaci na zbiór należący do dokładnego pokrycia,
	 * ten cstring wypisywany jako kolejne odpowiedzi. */
	char *odpowiedz= malloc ( sizeof (char) * (szer_max+1));
	
	/*	Pomocnicza tablica przechowująca uchwyty,
	 * do elementów, wykorzystywana w pokrycie() */
	obiekt **O = malloc ( sizeof(obiekt*) * szer_max);
	
	/*	Lista do kolejnych, nie ukrytych kolumn,
	 * jako atrybut zawiera numer kolumny i 
	 * doczepienie do innych kolumn oraz do elementów,
	 * w danej kolumnie - tworząc listę cykliczną */
	obiekt *kolumny = malloc ( sizeof (obiekt));
	
	/*	Trzyma wskaźniki na kolejne elemnty listy kolumny -
	 * pozwala znając numer kolumny, szybko przejść,
	 * do jej reprezentanta na liście */
	obiekt **tab_kolumna = malloc ( sizeof(obiekt*) * szer_max);
	
	/* Pomocnicze wskaźniki,
	 * wykorzystywane przy budowaniu
	 * struktur*/
	 
	obiekt *wsk=kolumny;
	obiekt *ostatni=kolumny;
	for(i=0;i<szer_max;i++)
		odpowiedz[i]=' ';
		
	kolumny ->war=-1;/*	Atrapa - element opisany 
						przez Knutha jako "h"*/
	for(i=0;i<szer_max;i++)
	{		
		/*	Alokuj pamięć na kolejny wierchołek */
		obiekt *tmp=  malloc ( sizeof (obiekt));
		tmp->war=i;
		
		/*	Twórz dowiązania */
		(*kolumna_start)[i]->gora= tmp;
		(*kolumna_koniec)[i]->dol= tmp;
		
		tmp->dol=(*kolumna_start)[i];
		tmp->gora=(*kolumna_koniec)[i];
		tab_kolumna[i]=tmp;
		
		/*	Idz dalej wskaźnikiem */
		wsk->prawo=tmp;
		wsk=wsk->prawo;
	}
	
	/*	Teraz zadbaj o cykliczność listy kolumn */
	wsk->prawo=kolumny ;
	
	kolumny ->lewo=wsk;
	
	wsk=kolumny->prawo ;
	ostatni = kolumny;

	/*	Utwórz dowiązania w drugą stronę -
	 * double list */
	while(wsk!=kolumny)
	{

		wsk->lewo=ostatni;
		ostatni=ostatni->prawo;
		wsk=wsk->prawo;
	}
	
	
	
	/*printf("\n");*/
	odpowiedz[szer_max]='\0';
	/*	Zacznij szukać dokładnego pokrycia */
	pokrycie(&kolumny, odpowiedz,  &tab_kolumna,kolumna_start, 0,&O, num);
	
	
	/*	Zwolnij pamięć */
	free(odpowiedz);	
	free(O);	
	free(num);	
	
	for(i=0; i< szer_max; i++)	
		free(tab_kolumna[i]);

	free(tab_kolumna);	
	free(kolumny);
}	


/*	Procedura "odkrywa" (uncover) ukrytą, przez
*	schowaj kolumne, wskazaną przec c
*	[in,out] c - wskaźnik na kolumne na liście
* 				kolumn (columna object z pracy Knutha)*/
void przywroc(obiekt **c)	
{
	/*	Skorzystaj z cykliczności */
	obiekt *i= (*c)->gora;
	while(i!=(*c))
	{
		
		obiekt *j=i->lewo;
		while(j!=i)
		{	/* Przywracaj połączenie 
				wewnątrz struktury */
			j->dol->gora=j;
			j->gora->dol=j;
			j=j->lewo;
		}
		
		
		i=i->gora;
	}
	
	/* Jeszcze przywróć kolumne
	 * do listy kolumn */
	(*c)->prawo->lewo=(*c);
	(*c)->lewo->prawo=(*c);
}			

/*	Procedura ukrywa (cover) wskazaną kolumne c.
*	[in,out] c - wskaźnik na kolumne na liście
* 				kolumn (columna object z pracy Knutha)*/
void schowaj(obiekt **c )	
{
	/*	Najpierw schowaj z listy kolumn */
	obiekt *i = (*c)->dol;
	(*c)->prawo->lewo=(*c)->lewo;
	(*c)->lewo->prawo=(*c)->prawo;

	/*	Teraz przesuwaj wewnętrzne dowiązania, 
	 * 	w celu ukrycia wskazanych elementów */
	while(i!=(*c))
	{
	
		obiekt *j=i->prawo;
		while(j!=i)
		{	
			j->dol->gora=j->gora;	
			j->gora->dol=j->dol;
			j=j->prawo;
		}
		
		i=i->dol;
	}
}	



/*	Funkcja znajdująca instancje rozwiązania.
*	Zasada działania jest rekurencyjna zgodna,
* 	z algorytmem przedstawionym przez Knutha,
*	w pracy, która przytoczyłem na początku kodu.
*	Funkcja metodą prób i błędów próbuje wszysctkich
* 	nie wykluczających się kombinacji.
*	Zawsze stara się pokryć pierwszą wolną kolumne,
*	rekurencja daje możlwiość backtracking - jeżeli
*	funkcja zapędza się w "ślepą uliczkę", to cofa
*	i próbuje innego wyboru - systematycznie, aż
*	sprawdzi wszystkie kombinacje.
*	Kolejne wywołania rekurecnji dostają wspólny obszar
* 	roboczy - odp i O.
*	[in,out] kolumny - lista kolumn (column object).
* 	[in,out] odp - wspólny dla kolejnych wywołań cstring,
* 				gdzie przechowywana ew. odpowiedź.
* 	[in,out] tab_kol - tablica trzyma wskaźniki 
* 				na kolejne elemnty listy kolumny -
*	 			pozwala, znając numer kolumny, szybko przejść
* 				do jej reprezentanta na liści
* 	[in,out] kolumna_start []- tablica przechowująca dowiązania
*					do początków kolejnych kolumn.
* 	[in] k - które wywołanie rekurencyjne.
* 	[in,out] O- tablica z aktualnie "zakwalifikowanymi" 
* 					elemantami.
* 	[in,out] num - tablica podobna do odp, jednak nie 
* 					przechowuje znaków, a numery wierszy, które
* 					aktualnie w dokładnym (może niepełnym) 
* 					pokryciu.
* 	@return - używane tylko do przerwania rekurencji. */			
int pokrycie(obiekt **kolumny, char *odp,
				obiekt **tab_kol[], obiekt **kolumna_start[], 
				int k, obiekt **O[], int num[])	
{
	/* Nazewnictwo zmiennych zgodne jak w pracy Knutha*/
	obiekt *c, *r;
	
	if((*kolumny)->prawo==(*kolumny))
	{/* Jeżeli wszystkie kolmny pokryte */
		printf("%s\n", odp);	/* Wypisz instancje odpowiedzi */
		return 0;/*	Zakończ wywołanie */
	}
	
	c=(*kolumny)->prawo;
	schowaj(&c);
	r= c->dol;
		
	/*	Pokryj wiersze mający wspólny el. z kolumną*/	
	while(r!=c)
	{
		obiekt *wskaznik, *j;
		(*O)[k]=r;/* Zapisz uchwyt */
		
		
		wskaznik= r;
		do/* Dodaj elementy z danego wiersza do odpowiedzi */
		{
			odp[wskaznik->x]=wskaznik->war;
			wskaznik=wskaznik->prawo;
		}while(r!=wskaznik);
		
		/*	Zapisz numer wiersza, zaliczanego do odp */
		num[k]=r->y;
		
		
		j= r->prawo;
		/*	Pokryj kolumny mający wspólny el. z 
		 * 	danym wierszem*/	
		while(j!=r)
		{
				
			obiekt *usun= (*tab_kol)[j->x];
			schowaj(&usun);
			j=j->prawo;
		}
		/*	Wywołaj dla kolejnej kolumny */
		pokrycie(kolumny, odp, tab_kol,kolumna_start ,k+1, O, num);
		
		r=(*O)[k]; c=(*tab_kol)[r->x];
		
		/*	Cofamy w rekurancji - 
		 * odkryj ukryte ,przez obieg pętli w której 
		 * jesteś, elementy*/
		j=r->lewo;
		while(j!=r)
		{	
			obiekt *rec =(*tab_kol)[j->x];	
			przywroc(&rec);
			j=j->lewo;
		}
		r=r->dol;
	}
	/*	Odkryj kolumne dla której całe wywołanie danej
	 *  funkcji, i na której działała powyższa pętla */
	przywroc(&c);
	
	return 0; /* Formalność */			
}
