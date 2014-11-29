#include <windows.h>
#include <SDL.h>
#include <GL/glu.h>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <math.h>
#include <time.h>
#define MAXV 15000      //maksimum vertex sayisi
#define MAXT 15000      //maksimum üçgen sayisi
#define MAX_DEPTH 20    //agacin maksimum derinligi
using namespace std;

enum {UNKNOWN, VISIBLE, INVISIBLE};

class vertex    // vertexler için tanimlanmis sinif
{
    float x;
    float y;
    float z;        // vertex'in koordinatlari
    int visibility; // vertex'in görünülebilirligi
    int number;     // vertex'in numarasi
public:
    vertex(){ visibility=UNKNOWN;  }
    vertex(float inx, float iny, float inz)                 //constructor
    {
        x=inx;
        y=iny;
        z=inz;
    }
    void setVertex(float inx, float iny, float inz)         // vertex için set-get metodlari
    {
        x=inx;
        y=iny;
        z=inz;
    }
    void setVertexX(float input)
    {
        x=input;
    }
    void setVertexY(float input)
    {
        y=input;
    }
    void setVertexZ(float input)
    {
        z=input;
    }
    void setVisibility(int vis)
    {
        visibility=vis;
    }
    void setNumber(int num)
    {
        number=num;
    }
    float getVertexX()
    {
        return x;
    }
    float getVertexY()
    {
        return y;
    }
    float getVertexZ()
    {
        return z;
    }
    int getVisibility()
    {
        return visibility;
    }
    int getNumber()
    {
        return number;
    }
};

class triangle  // üçgen yüzler için tanimlanmis sinif
{
    int first;
    int second;
    int third;      // üçgenin olusturan vertexlerin numaralari
    int order;      // üçgenin hangi objeye ait oldugu
    bool visibility; //üçgenin görünülebilirligi
public:
    triangle(){  visibility=true;   }
    triangle(int f, int s, int t, int o)    // constructor
    {
        first=f;
        second=s;
        third=t;
        order=o;
    }
    void setTriangleFirst(int i)        // set-get metodlari
    {
        first=i;
    }
    void setTriangleSecond(int i)
    {
        second=i;
    }
    void setTriangleThird(int i)
    {
        third=i;
    }
    void setTriangleOrder(int i)
    {
        order=i;
    }
    void setTriangleVisibility(bool b)
    {
        visibility=b;
    }
    int getTriangleFirst()
    {
        return first;
    }
    int getTriangleSecond()
    {
        return second;
    }
    int getTriangleThird()
    {
        return third;
    }
    int getTriangleOrder()
    {
        return order;
    }
    bool getTriangleVisibility()
    {
        return visibility;
    }
};

struct ray      // isin için olusturulmus sinif
{
    vertex start;   // isinin gönderildigi vertexi
    vertex end;     // üzerine isin gönderilen vertex
    int* regionarray;   // isinin geçtigi bölgeleri tutan dizi
    int arraysize;      // bu dizinin boyutu
};

struct node         //agaç dügümü için olusturulmus sinif
{
    vertex value;   // dügümün vertex degeri
    node *left;     // sol çocuk dügüm
    node *right;    // sag çocuk dügüm
    int nodesize;   // dügümün altinda kaç dügüm oldugunun göstergesi
    int region;     // dügümün isaret ettigi bölgenin numarasi: kök dügümün numarasi 1'dir ve bir dügümün numarasi k ise sol dügümün numarasi 2k, sag dügümün numarasi 2k+1'dir.
    int *trioarray; // yaprak dügümde bulunan üçgenleri tutan dizi
    int arraysize;  // bu dizinin boyutu
};

        vertex viewPoint(0,0,0);            // bakis noktasinin koordinatlari
        vertex obj1location(0,0,-80);       // ilk objenin koordinatlari
        vertex obj2location(10,0,-100);       // ikinci objenin koordinatlari
        int mode=1;
        int raycount=0;                     // toplam gönderilen isin sayisi
        int intersection=0;                 // toplam uygulanan kesisim testi sayisi
        vertex *vertexes=new vertex[MAXV];  // vertexleri tutan dizi
        int leafcount=0;                    // toplam yaprak dügüm sayisi
        float normals[6][3];                // obje dosyasindaki normalleri tutan dizi (kullanilmilyor)
        triangle *trios=new triangle[MAXT]; // üçgen yüzeyleri tutan dizi
        int triosize=0;                     // toplam üçgen yüzey sayisi
        int vertexsize=0;                   // toplam vertex sayisi
        int vertexsize1=0;                  // ilk objenin vertex sayisi
        int control=0;

void mergeArrays(vertex *array1, vertex *array2, int min, int mid, int max, int mode)
{
    int a=min;
    int b=min;
    int c=mid+1;
    int d;
    while((b<=mid)&&(c<=max))
    {
        if(mode==0)
        {
            if(array1[b].getVertexX()<=array1[c].getVertexX())
            {
                array2[a]=array1[b];
                b++;
            }
            else
            {
                array2[a]=array1[c];
                c++;
            }
        a++;
        }
        else if(mode==1)
        {
            if(array1[b].getVertexY()<=array1[c].getVertexY())
            {
                array2[a]=array1[b];
                b++;
            }
            else
            {
                array2[a]=array1[c];
                c++;
            }
        a++;
        }
        else if(mode==2)
        {
          if(array1[b].getVertexZ()<=array1[c].getVertexZ())
            {
            array2[a]=array1[b];
            b++;
            }
            else
            {
            array2[a]=array1[c];
            c++;
            }
        a++;
        }

    }
    if(b>mid)
    {
        for(d=c;d<=max;d++)
        {
        array2[a]=array1[d];
        a++;
        }
    }
    else
    {
        for(d=b;d<=mid;d++)
        {
        array2[a]=array1[d];
        a++;
        }
    }
    for(d=min;d<=max;d++)
    {
        array1[d]=array2[d];
    }
}

void mergeSort(vertex *array1, vertex *array2, int min, int max, int mode)  // vertexleri siralamak için kullanilan mergeSort fonksiyonu
// mod 0 ise x'e göre, 1 ise y'ye göre, 2 ise z'ye göre siralama yapar.
{
    if(min<max)
    {
        int mid=(min+max)/2;
        mergeSort(array1,array2,min,mid,mode);
        mergeSort(array1,array2,mid+1,max,mode);
        mergeArrays(array1,array2,min,mid,max,mode);
    }
}

void buildkdtree(vertex* array1, node *n, int depth, int size, int reg)     // agaç yapisini olusturan fonksiyon
// giris degerleri: vertex dizisi, bulunulan dügüm, dügümün derinligi, dügümün altinda kalacak dügüm sayisi ve dügümün numarasi
{
    if (size>0)     // yaprak degil ise
    {
        vertex* leftarray=new vertex[size/2+1];     // sol çocuk dügümün tutacagi vertex dizisi
        vertex* rightarray=new vertex[(size+1)/2];  // sag çocuk dügümün tutacagi vertex dizisi
        vertex* temp=new vertex[size+1];            // siralama için kullanilacak geçici dizi
        if((depth%3)==0)                            // derinlik 0(mod 3) ise
        {
            mergeSort(array1,temp,0,size,0);        // x'e göre sirala
        }
        else if((depth%3)==1)                       // derinlik 1(mod 3) ise
        {
            mergeSort(array1,temp,0,size,1);        // y'ye göre sirala
        }
        else                                        // derinlik 2(mod 3) ise
        {
            mergeSort(array1,temp,0,size,2);        // z'ye göre sirala
        }
        n->region=reg;                              // dügümün numarasi
        n->nodesize=size;                           // dügümün boyutu
        n->value=array1[size/2];                    // dügümün degeri (medyan)
        int a;
        for(a=0;a<=size/2;a++)
        {
            leftarray[a]=array1[a];                 //siralanmis dizinin küçük olan yarisi sol çocuk dügümün dizisini olusturur.
        }
        for(a=size/2+1;a<size+1;a++)
        {
            rightarray[a-size/2-1]=array1[a];       //siralanmis dizinin diger yarisi da sag çocuk dügümün dizisini olusturur.
        }
        n->left=new node;
        buildkdtree(leftarray, n->left, depth+1, size/2, 2*reg);  // ayni islemler sol çocuk için de tekrarlanir.
        n->right=new node;
        buildkdtree(rightarray, n->right, depth+1, (size+1)/2-1, 2*reg+1);  // ayni islemler sag cocuk için de tekrarlanir.
    }
    else if(size==0)        // yaprak ise
    {
        n->trioarray=new int[MAXT]; // yaprak dügümün üçgen dizisi
        n->value=array1[0];         // dügümün degeri
        n->arraysize=0;             // dizinin boyutu
        n->region=reg;              // dügümün numarasi
        n->nodesize=size;           // dügümün boyutu sifir
        leafcount++;                // yaprak sayaci
    }
}

void findRegion(vertex firstv, vertex secondv, vertex thirdv, int firstt, int secondt, int thirdt, node *n, int depth, int counter) //verilen üçgenin hangi bölgelerde kaldigini bulan fonksiyon
// giris degerleri: üçgeni olusturan üç vertex, bu üç vertexin numaralari, bulunulan dügüm, dügümün derinligi, üçgenin numarasi
{
    if(n->nodesize>0)   // dügüm yaprak degil ise
    {
        if((depth%3)==0)  // x boyutuna göre bak
        {
            if((firstv.getVertexX()<=n->value.getVertexX())&&(secondv.getVertexX()<=n->value.getVertexX())&&(thirdv.getVertexX()<=n->value.getVertexX())) // üçgenin tüm vertexleri dügümün degerinin solunda kaliyorsa sadece sol dügüme ilerle
            {
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->left,depth+1,counter);
            }
            else if((firstv.getVertexX()>=n->value.getVertexX())&&(secondv.getVertexX()>=n->value.getVertexX())&&(thirdv.getVertexX()>=n->value.getVertexX())) // üçgenin tüm vertexleri dügümün degerinin saginda kaliyorsa sadece sag dügüme ilerle
            {
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->right,depth+1,counter);
            }
            else // yoksa her iki dügüme de ilerle
            {
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->left,depth+1,counter);
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->right,depth+1,counter);
            }
        }
        else if((depth%3)==1) // y boyutuna göre bak
        {
            if((firstv.getVertexY()<=n->value.getVertexY())&&(secondv.getVertexY()<=n->value.getVertexY())&&(thirdv.getVertexY()<=n->value.getVertexY()))
            {
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->left,depth+1,counter);
            }
            else if((firstv.getVertexY()>=n->value.getVertexY())&&(secondv.getVertexY()>=n->value.getVertexY())&&(thirdv.getVertexY()>=n->value.getVertexY()))
            {
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->right,depth+1,counter);
            }
            else
            {
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->left,depth+1,counter);
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->right,depth+1,counter);
            }
        }
        else    // z boyutuna göre bak
        {
            if((firstv.getVertexZ()<=n->value.getVertexZ())&&(secondv.getVertexZ()<=n->value.getVertexZ())&&(thirdv.getVertexZ()<=n->value.getVertexZ()))
            {
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->left,depth+1,counter);
            }
            else if((firstv.getVertexZ()>=n->value.getVertexZ())&&(secondv.getVertexZ()>=n->value.getVertexZ())&&(thirdv.getVertexZ()>=n->value.getVertexZ()))
            {
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->right,depth+1,counter);
            }
            else
            {
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->left,depth+1,counter);
                findRegion(firstv,secondv,thirdv,firstt,secondt,thirdt,n->right,depth+1,counter);
            }
        }
    }
    else    // dügüm yaprak ise
    {
        n->trioarray[n->arraysize]=counter; // üçgeni dügümün dizisine ekle
        n->arraysize++;                     // eleman sayisini bir arttir
    }
}
void setRegion(vertex* vertexarray, triangle* trarray, node *n, int max) // tüm üçgenlerin bulunduklari bölgeleri belirleyen fonksiyon
//giris degerleri: vertex dizisi, üçgen dizisi, kök dügüm, maksimum üçgen sayisi
{
    for(int a=0;a<max;a++)
    {
        findRegion(vertexarray[trarray[a].getTriangleFirst()-1],vertexarray[trarray[a].getTriangleSecond()-1],vertexarray[trarray[a].getTriangleThird()-1],trarray[a].getTriangleFirst(),trarray[a].getTriangleSecond(),trarray[a].getTriangleThird(),n,0,a);
    }
}

void findRayRegion(ray *ray1, node *n, int depth) // verilen isinin hangi bölgelerden geçtigini bulan fonksiyon
//giris degerleri: isin, bulunulan dügüm, bulunulan dügümün derinligi
{
        if(n->nodesize>0)   // yaprak degil ise
        {
            if((depth%3)==0) // x boyutuna bak
            {
                if((ray1->start.getVertexX()<=n->value.getVertexX())&&(ray1->end.getVertexX()<=n->value.getVertexX())) // isin dügümün solunda kaliyorsa sol dügümden ilerle
                {
                    findRayRegion(ray1, n->left, depth+1);
                }
                else if((ray1->start.getVertexX()>=n->value.getVertexX())&&(ray1->end.getVertexX()>=n->value.getVertexX())) // isin dügümün saginda kaliyorsa sag dügümden ilerle
                {
                    findRayRegion(ray1, n->right, depth+1);
                }
                else    // yoksa her iki dügümden de ilerle
                {
                    findRayRegion(ray1, n->left, depth+1);
                    findRayRegion(ray1, n->right, depth+1);
                }
            }
            else if((depth%3)==1) // y boyutuna bak
            {
                if((ray1->start.getVertexY()<=n->value.getVertexY())&&(ray1->end.getVertexY()<=n->value.getVertexY()))
                {
                    findRayRegion(ray1, n->left, depth+1);
                }
                else if((ray1->start.getVertexY()>=n->value.getVertexY())&&(ray1->end.getVertexY()>=n->value.getVertexY()))
                {
                    findRayRegion(ray1, n->right, depth+1);
                }
                else
                {
                    findRayRegion(ray1, n->left, depth+1);
                    findRayRegion(ray1, n->right, depth+1);
                }
            }
            else    // z boyutuna bak
            {
                if((ray1->start.getVertexZ()<=n->value.getVertexZ())&&(ray1->end.getVertexZ()<=n->value.getVertexZ()))
                {
                    findRayRegion(ray1, n->left, depth+1);
                }
                else if((ray1->start.getVertexZ()>=n->value.getVertexZ())&&(ray1->end.getVertexZ()>=n->value.getVertexZ()))
                {
                    findRayRegion(ray1, n->right, depth+1);
                }
                else
                {
                    findRayRegion(ray1, n->left, depth+1);
                    findRayRegion(ray1, n->right, depth+1);
                }
            }
        }
        else    // yaprak ise
        {
            ray1->regionarray[ray1->arraysize]=n->region;   // isinin geçtigi bölge dizisine o bölgeyi ekle
            ray1->arraysize++;                              // eleman sayisini 1 arttir
        }
}

bool insideTest(vertex v1, vertex v2, vertex v3, vertex ip, vertex normal) // isinin üçgenle kesistigi noktanin üçgenin içinde kalip kalmadigini kontrol eden fonksiyon
// giris degerleri: üçgenin verteksleri, kesisim noktasi, normal vektörü
// buradaki formüller için http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-9-ray-triangle-intersection/ray-triangle-intersection-geometric-solution adresini kullandim.
{
    //kenarlar
    vertex e1(v2.getVertexX()-v1.getVertexX(),v2.getVertexY()-v1.getVertexY(),v2.getVertexZ()-v1.getVertexZ());
    vertex e2(v3.getVertexX()-v2.getVertexX(),v3.getVertexY()-v2.getVertexY(),v3.getVertexZ()-v2.getVertexZ());
    vertex e3(v1.getVertexX()-v3.getVertexX(),v1.getVertexY()-v3.getVertexY(),v1.getVertexZ()-v3.getVertexZ());
    //kesisim noktasinin üçgenin köseleriyle olusturdugu vektörler
    vertex c1(ip.getVertexX()-v1.getVertexX(),ip.getVertexY()-v1.getVertexY(),ip.getVertexZ()-v1.getVertexZ());
    vertex c2(ip.getVertexX()-v2.getVertexX(),ip.getVertexY()-v2.getVertexY(),ip.getVertexZ()-v2.getVertexZ());
    vertex c3(ip.getVertexX()-v3.getVertexX(),ip.getVertexY()-v3.getVertexY(),ip.getVertexZ()-v3.getVertexZ());
    //cross product
    vertex cr1(e1.getVertexY()*c1.getVertexZ()-e1.getVertexZ()*c1.getVertexY(),e1.getVertexZ()*c1.getVertexX()-e1.getVertexX()*c1.getVertexZ(),e1.getVertexX()*c1.getVertexY()-e1.getVertexY()*c1.getVertexX());
    vertex cr2(e2.getVertexY()*c2.getVertexZ()-e2.getVertexZ()*c2.getVertexY(),e2.getVertexZ()*c2.getVertexX()-e2.getVertexX()*c2.getVertexZ(),e2.getVertexX()*c2.getVertexY()-e2.getVertexY()*c2.getVertexX());
    vertex cr3(e3.getVertexY()*c3.getVertexZ()-e3.getVertexZ()*c3.getVertexY(),e3.getVertexZ()*c3.getVertexX()-e3.getVertexX()*c3.getVertexZ(),e3.getVertexX()*c3.getVertexY()-e3.getVertexY()*c3.getVertexX());
    if(normal.getVertexX()*cr1.getVertexX()+normal.getVertexY()*cr1.getVertexY()+normal.getVertexZ()*cr1.getVertexZ()<=0)
    {
        return false;   // kesmiyor
    }
    else
    {
        if(normal.getVertexX()*cr2.getVertexX()+normal.getVertexY()*cr2.getVertexY()+normal.getVertexZ()*cr2.getVertexZ()<=0)
        {
            return false;   // kesmiyor
        }
        else
        {
           if(normal.getVertexX()*cr3.getVertexX()+normal.getVertexY()*cr3.getVertexY()+normal.getVertexZ()*cr3.getVertexZ()<=0)
           {
               return false;    // kesmiyor
           }
           else
           {
               return true; // kesiyor
           }
        }
    }
}

bool findIntersection(ray* ray1, vertex v1, vertex v2, vertex v3)   // üçgenin verilen isini kesip kesmedigini bulan fonksiyon
// giris degerleri: isin ve üçgenin vertexleri
// formülleri yine http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-9-ray-triangle-intersection/ray-triangle-intersection-geometric-solution adresinden aldim.
{
    //üçgenin herhangi iki kenari
    vertex e1(v2.getVertexX()-v1.getVertexX(),v2.getVertexY()-v1.getVertexY(),v2.getVertexZ()-v1.getVertexZ());
    vertex e2(v3.getVertexX()-v1.getVertexX(),v3.getVertexY()-v1.getVertexY(),v3.getVertexZ()-v1.getVertexZ());
    // normal vektörü
    vertex normal(e1.getVertexY()*e2.getVertexZ()-e2.getVertexY()*e1.getVertexZ(),e2.getVertexX()*e1.getVertexZ()-e1.getVertexX()*e2.getVertexZ(),e1.getVertexX()*e2.getVertexY()-e2.getVertexX()*e1.getVertexY());
    // isinin uzunlugu
    float magnitude=sqrt((ray1->end.getVertexX()-ray1->start.getVertexX())*(ray1->end.getVertexX()-ray1->start.getVertexX())+(ray1->end.getVertexY()-ray1->start.getVertexY())*(ray1->end.getVertexY()-ray1->start.getVertexY())+(ray1->end.getVertexZ()-ray1->start.getVertexZ())*(ray1->end.getVertexZ()-ray1->start.getVertexZ()));
    // isinin dogrultu vektörü
    vertex direction((ray1->end.getVertexX()-ray1->start.getVertexX())/magnitude,(ray1->end.getVertexY()-ray1->start.getVertexY())/magnitude,(ray1->end.getVertexZ()-ray1->start.getVertexZ())/magnitude);
    // üçgenin orijine uzakligi
    float distance=-(normal.getVertexX()*v1.getVertexX()+normal.getVertexY()*v1.getVertexY()+normal.getVertexZ()*v1.getVertexZ());
    // pay
    float a=normal.getVertexX()*ray1->start.getVertexX()+normal.getVertexY()*ray1->start.getVertexY()+normal.getVertexZ()*ray1->start.getVertexZ()+distance;
    // payda
    float b=normal.getVertexX()*direction.getVertexX()+normal.getVertexY()*direction.getVertexY()+normal.getVertexZ()*direction.getVertexZ();
    if(b!=0)
    {
        float t=-a/b;
        vertex intersection(ray1->start.getVertexX()+t*direction.getVertexX(),ray1->start.getVertexY()+t*direction.getVertexY(),ray1->start.getVertexZ()+t*direction.getVertexZ());
        return insideTest(v1,v2,v3,intersection,normal);
    }
    return false;
}

float vertexMin(float v1, float v2, float v3) // üç vertexten hangisinin z degerinin en küçük oldugunu bulan fonksiyon
{
    if(v1<v2)
    {
        if(v1<v3)
        {
            return v1;
        }
        else
        {
            return v3;
        }
    }
    else
    {
        if(v2<v3)
        {
            return v2;
        }
        else
        {
            return v3;
        }
    }
}

bool checkRay(ray* ray1, vertex* vertexarray, triangle* trarray, int counter, node *n) // verilen isinin herhangi bir üçgen tarafindan kesilip kesilmedigini bulan fonksiyon
// giris degerleri: isin, vertex ve üçgen dizileri, sayaç, kök dügüm
{
    findRayRegion(ray1, n, 0); // öncelikle isinin geçtigi bölgeler bulunur
    int a;
    int b;
    int c;
    bool intersect=false; // kesme kontrolü
    node *nd=n; // agaç üzerinde gezmek için kullanilan pointer
    int* array1=new int[MAX_DEPTH]; // agaç üzerinde istenilen bölgeye ulasmak için izlenen yolu gösteren dizi
    int arraysize=0;    // dizinin boyutu
    bool isChecked[triosize];   // ayni üçgen birden fazla bölgede bulunuyorsa o üçgen için tekrar tekrar isin kesisim isleminin uygulanmasini engelleyen dizi
    for(a=0;a<triosize;a++)
    {
        isChecked[a]=false; // basta hiçbir üçgen kontrol edilmedigi için hepsi false olacak.
    }
    for(a=0;a<ray1->arraysize;a++) // isinin geçtigi her bölge kontrol edilecek
    {
        b=ray1->regionarray[a]; // isinin geçtigi bölgenin numarasi
        while(b!=1) // bölgenin numarasindan köke hangi yol ile gidildigi bulunabilir
        {
            if((b%2)==0) // numara çift ise sol dügümde bulunuluyordur
            {
                b=b/2;
                array1[arraysize]=0; // 0, sol dügüme gidisi temsil etmektedir.
                arraysize++;
            }
            else    // numara tek ise sag dügümde bulunuluyordur
            {
                b=b/2;
                array1[arraysize]=1; // 1, sag dügüme gidisi temsil etmektedir.
                arraysize++;
            }
        }
        for(c=arraysize-1;c>=0;c--) // yol ile alakali dizi olustuktan sonra bu dizi tersten okunarak kökten numarasi belirtilen bölgeye ulasilabilir.
        {
            if(array1[c]==0)    // 0 ise sol dügüm
            {
                nd=nd->left;
            }
            else                // 1 ise sag dügüm
            {
                nd=nd->right;
            }
        }
        for(c=0;c<nd->arraysize;c++)    // isin bölgede bulunan üçgenler ile kontrol edilir
        {
            if(trarray[nd->trioarray[c]].getTriangleVisibility()) // üçgenin görülemedigi bulunmussa kontrole gerek yoktur.
            {
                if(isChecked[nd->trioarray[c]]==false) // üçgen ayni isin için baska bir bölgede kontrol edilmisse tekrar kontrole gerek yoktur.
                {
                    isChecked[nd->trioarray[c]]=true;  // üçgen kontrol edildi
                    // üçgenin vertexleri
                    vertex v1=vertexarray[trarray[nd->trioarray[c]].getTriangleFirst()-1];
                    vertex v2=vertexarray[trarray[nd->trioarray[c]].getTriangleSecond()-1];
                    vertex v3=vertexarray[trarray[nd->trioarray[c]].getTriangleThird()-1];
                    // üçgenin kendisiyle kontrolünün engellenmesi
                    if((v1.getVertexX()!=vertexarray[trarray[counter].getTriangleFirst()-1].getVertexX())||(v1.getVertexY()!=vertexarray[trarray[counter].getTriangleFirst()-1].getVertexY())||(v1.getVertexZ()!=vertexarray[trarray[counter].getTriangleFirst()-1].getVertexZ())||(v2.getVertexX()!=vertexarray[trarray[counter].getTriangleSecond()-1].getVertexX())||(v2.getVertexY()!=vertexarray[trarray[counter].getTriangleSecond()-1].getVertexY())||(v2.getVertexZ()!=vertexarray[trarray[counter].getTriangleSecond()-1].getVertexZ())||(v3.getVertexX()!=vertexarray[trarray[counter].getTriangleThird()-1].getVertexX())||(v3.getVertexY()!=vertexarray[trarray[counter].getTriangleThird()-1].getVertexY())||(v3.getVertexZ()!=vertexarray[trarray[counter].getTriangleThird()-1].getVertexZ()))
                    {
                        if(vertexMin(v1.getVertexZ(),v2.getVertexZ(),v3.getVertexZ())>ray1->end.getVertexZ()) // isinin gönderildigi noktanin arkasinda kalan üçgenler isini kesemeyeceginden kontrole gerek yoktur.
                        {
                            intersection++; // kesisim kontrolü sayaci
                            intersect=findIntersection(ray1,v1,v2,v3); // kesisim kontrolü
                        }
                        if(intersect) // kesisme varsa
                        {
                            vertexarray[ray1->end.getNumber()].setVisibility(INVISIBLE); // isin kesiliyorsa o vertex görünemez olarak belirlenir
                            break; // o isin için test bitirilir
                        }
                    }
                }
            }
        }
        if(intersect) // kesisme varsa
        {
            break; // o isin için test bitirilir
        }
        else // kesisme yoksa
        {
            arraysize=0; // islemler diger bölgeler için tekrarlanir
            nd=n;
        }
    }
    if(!intersect) // isinin geçtigi tüm bölgeler kontrol edildikten sonra hiç kesisme olmadiysa
    {
        vertexarray[ray1->end.getNumber()].setVisibility(VISIBLE); // vertex görünür olarak belirlenir
    }
    return intersect; // kesisim olup olmadigi
}

void checkVisibility(vertex vp, vertex* vertexarray, triangle* trarray, node *n, int max) // üçgenlerin görünüp görünmedigini bulan fonksiyon
{
    bool finished=false; // bitis sarti
    int a=0;
    ray *ray1; // gönderilecek isin
    ray1->regionarray=new int[MAXT];
    while(finished==false)
    {
        if(a>=max)
        {
            finished=true;
        }
        else
        {
            ray1->arraysize=0;
            ray1->start=vp;
            if(vertexarray[trarray[a].getTriangleFirst()-1].getVisibility()==UNKNOWN) // üçgenin ilk vertexinin görülebilirligi kontrol edilmediyse kontrol et
            {
                ray1->end=vertexarray[trarray[a].getTriangleFirst()-1];
                raycount++; // isin sayaci
                checkRay(ray1, vertexarray, trarray, a, n); // isini kontrol et
            }
            if(vertexarray[trarray[a].getTriangleSecond()-1].getVisibility()==UNKNOWN) // ikinci vertex
            {
                ray1->end=vertexarray[trarray[a].getTriangleSecond()-1];
                raycount++;
                checkRay(ray1, vertexarray, trarray, a, n);
            }
            if(vertexarray[trarray[a].getTriangleThird()-1].getVisibility()==UNKNOWN) // üçüncü vertex
            {
                ray1->end=vertexarray[trarray[a].getTriangleThird()-1];
                raycount++;
                checkRay(ray1, vertexarray, trarray, a, n);
            }
            //üç vertex de görülemiyorsa
            if((vertexarray[trarray[a].getTriangleFirst()-1].getVisibility()==INVISIBLE)&&(vertexarray[trarray[a].getTriangleSecond()-1].getVisibility()==INVISIBLE)&&(vertexarray[trarray[a].getTriangleThird()-1].getVisibility()==INVISIBLE))
            {
                trarray[a].setTriangleVisibility(false); // üçgen görülemiyor
            }
            a++;
        }
    }
}

void drawVertexes() // üçgenleri ekrana çizme fonksiyonu
{
    int a;
    int visible=0; // görülebilen üçgen sayaci
    int invisible=0; // görülemeyen üçgen sayaci
    clock_t t;
    t=clock();
    for(a=0;a<triosize;a++)
    {
        if(trios[a].getTriangleVisibility()) // üçgen görülüyorsa
        {
            visible++;
            if(trios[a].getTriangleOrder()==1) // üçgen ilk objeye aitse
            {
                glColor3f(1.0,0.0,0.0); // üçgenin rengi
                glBegin(GL_TRIANGLES);
                glVertex3f(vertexes[trios[a].getTriangleFirst()-1].getVertexX(),vertexes[trios[a].getTriangleFirst()-1].getVertexY(),vertexes[trios[a].getTriangleFirst()-1].getVertexZ());
                glVertex3f(vertexes[trios[a].getTriangleSecond()-1].getVertexX(),vertexes[trios[a].getTriangleSecond()-1].getVertexY(),vertexes[trios[a].getTriangleSecond()-1].getVertexZ());
                glVertex3f(vertexes[trios[a].getTriangleThird()-1].getVertexX(),vertexes[trios[a].getTriangleThird()-1].getVertexY(),vertexes[trios[a].getTriangleThird()-1].getVertexZ());
                glEnd();
            }
            else if(trios[a].getTriangleOrder()==2) // üçgen ikinci objeye aitse
            {
                glColor3f(0.8,0.6,0.0); // üçgenin rengi
                glBegin(GL_TRIANGLES);
                glVertex3f(vertexes[trios[a].getTriangleFirst()-1].getVertexX(),vertexes[trios[a].getTriangleFirst()-1].getVertexY(),vertexes[trios[a].getTriangleFirst()-1].getVertexZ());
                glVertex3f(vertexes[trios[a].getTriangleSecond()-1].getVertexX(),vertexes[trios[a].getTriangleSecond()-1].getVertexY(),vertexes[trios[a].getTriangleSecond()-1].getVertexZ());
                glVertex3f(vertexes[trios[a].getTriangleThird()-1].getVertexX(),vertexes[trios[a].getTriangleThird()-1].getVertexY(),vertexes[trios[a].getTriangleThird()-1].getVertexZ());
                glEnd();
            }
        }
        else
        {
            invisible++;
        }
    }
    if(control==0)
    {
        t=clock()-t;
        FILE* out2=fopen("clock.txt","a+"); // fonksiyonların calismasinin ne kadar süre aldigi
        fprintf(out2,"drawVertexes: %f seconds\n",((float)t)/CLOCKS_PER_SEC);
        control++;
    }
    FILE* out=fopen("vertexes.txt","w+");
    fprintf(out,"gorunen yuz: %d\n",visible);
    fprintf(out,"gorunmeyen yuz: %d\n",invisible);
    fprintf(out, "toplam yuz: %d\n",visible+invisible);
    fprintf(out, "\nisin: %d\n", raycount);
    fprintf(out, "isin / yuz: %f\n", ((float)raycount/(float)triosize));
    fprintf(out, "kesisim testi: %d\n", intersection);
    fprintf(out, "kesisim testi / yuz: %f\n", ((float)intersection/(float)triosize));
    fclose(out);
}

void init()
{
        // baslangiç fonksiyonlari
        glClearColor(0.0,0.0,0.0,1.0);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45,640.0/480.0,1.0,500.0);
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
}

void display()
{
        //görüntü fonksiyonlari
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(0,0,0);
        drawVertexes();
}

int main(int argc, char** argv)
{
        // baslangiç degerleri
        SDL_Init(SDL_INIT_EVERYTHING);
        SDL_Surface *screen;
        screen = SDL_SetVideoMode(1024, 800, 8, SDL_SWSURFACE|SDL_OPENGL);
        bool running = true;
        const int FPS = 30;
        Uint32 start;
        SDL_Event event;
        int tempi;
        float tempf;
        char tempc;
        int na=0,nb=0;
        char buf[256];
        clock_t t;
        FILE *in1;
        FILE *in2;
        if(!(in1=fopen("teddy.obj","r")))
        {
                return -1;
        }
        while(!feof(in1)) // ilk objedeki degerlerin okunmasi
        {
            fscanf(in1,"%c",&buf[0]);
          if(buf[0]=='v') // ilk karakter v ise
            {
            fscanf(in1,"%c",&buf[1]);
            if(buf[1]=='n') // ikinci karakter n ise normal vektörüdür (gerceklenmedi)
            {
              /*fscanf(in1,"%f",&normals[na][nb]);
              nb++;
              fscanf(in1,"%f",&normals[na][nb]);
              nb++;
              fscanf(in1,"%f",&normals[na][nb]);
              nb=0;
              na++;
              fscanf(in1,"%c",&tempc);*/
            }
            else // degilse vertextir
            {
                fscanf(in1,"%f",&tempf);
                vertexes[vertexsize].setVertexX(tempf+obj1location.getVertexX()-viewPoint.getVertexX());
                fscanf(in1,"%f",&tempf);
                vertexes[vertexsize].setVertexY(tempf+obj1location.getVertexY()-viewPoint.getVertexY());
                fscanf(in1,"%f",&tempf);
                vertexes[vertexsize].setVertexZ(tempf+obj1location.getVertexZ()-viewPoint.getVertexZ());
                vertexes[vertexsize].setNumber(vertexsize);
                vertexsize++;
                vertexsize1++;
                fscanf(in1,"%c",&tempc);
            }
            }
            else if(buf[0]=='f') // ilk karakter f ise üçgendir (face)
            {
               fscanf(in1,"%d",&tempi);
               trios[triosize].setTriangleFirst(tempi);
               fscanf(in1,"%d",&tempi);
               trios[triosize].setTriangleSecond(tempi);
               fscanf(in1,"%d",&tempi);
               trios[triosize].setTriangleThird(tempi);
               trios[triosize].setTriangleOrder(1);
               triosize++;
            }

          }
        fclose(in1);
        if(!(in2=fopen("teddy.obj","r")))
        {
                return -1;
        }
        while(!feof(in2)) // ikinci obje degerleri
        {
            fscanf(in2,"%c",&buf[0]);
          if(buf[0]=='v')
            {
            fscanf(in2,"%c",&buf[1]);
            if(buf[1]=='n')
            {/*
              fscanf(in2,"%f",&normals[na][nb]);
              nb++;
              fscanf(in2,"%f",&normals[na][nb]);
              nb++;
              fscanf(in2,"%f",&normals[na][nb]);
              nb=0;
              na++;
            */
            }
            else
            {
                fscanf(in2,"%f",&tempf);
                vertexes[vertexsize].setVertexX(tempf+obj2location.getVertexX()-viewPoint.getVertexX());
                fscanf(in2,"%f",&tempf);
                vertexes[vertexsize].setVertexY(tempf+obj2location.getVertexY()-viewPoint.getVertexY());
                fscanf(in2,"%f",&tempf);
                vertexes[vertexsize].setVertexZ(tempf+obj2location.getVertexZ()-viewPoint.getVertexZ());
                vertexes[vertexsize].setNumber(vertexsize);
                vertexsize++;
                fscanf(in2,"%c",&tempc);
            }
            }
            else if(buf[0]=='f')
            {
               fscanf(in2,"%d",&tempi);
               trios[triosize].setTriangleFirst(tempi+vertexsize1);
               fscanf(in2,"%d",&tempi);
               trios[triosize].setTriangleSecond(tempi+vertexsize1);
               fscanf(in2,"%d",&tempi);
               trios[triosize].setTriangleThird(tempi+vertexsize1);
               trios[triosize].setTriangleOrder(2);
               triosize++;
            }
          }
        fclose(in2);
        FILE *out=fopen("clock.txt","w+"); // fonksiyonların calismasinin ne kadar süre aldigi
        if(mode==1)
        {
            int a;
            vertex *vertexes2=new vertex[MAXV]; // vertex dizisinin kopyasi
            for(a=0;a<vertexsize;a++)
            {
                vertexes2[a]=vertexes[a];
            }
            node* root=new node;
            t=clock();
            buildkdtree(vertexes2,root,0,vertexsize-1,1); // agaç yapisinin olusturulmasi
            t=clock()-t;
            fprintf(out,"buildkdtree: %f seconds\n",((float)t)/CLOCKS_PER_SEC);
            setRegion(vertexes,trios,root,triosize); // üçgenlerin bölgelerinin belirlenmesi
            t=clock()-t;
            fprintf(out,"setRegion: %f seconds\n",((float)t)/CLOCKS_PER_SEC);
            checkVisibility(viewPoint, vertexes, trios, root, triosize); // üçgenlerin görülebilirliginin kontrolü
            t=clock()-t;
            fprintf(out,"checkVisibility: %f seconds\n",((float)t)/CLOCKS_PER_SEC);
            fclose(out);
        }
        init();
        while(running) {
                start = SDL_GetTicks();
                while(SDL_PollEvent(&event)) {
                        switch(event.type) {
                                case SDL_QUIT:
                                        running = false;
                                        break;
                        }
                }

                display();
                SDL_GL_SwapBuffers();
                if(1000/FPS > SDL_GetTicks()-start)
                        SDL_Delay(1000/FPS-(SDL_GetTicks()-start));
        }
        SDL_Quit();
        return 0;
}
