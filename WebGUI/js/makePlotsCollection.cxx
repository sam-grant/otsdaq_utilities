{
 gROOT->Reset() ;
 
 int    nch =  25 ;
 double xmi = -10 ;
 double xma =  10 ;
 
 double peak  = 0 ;
 double width = 2 ;
 
 TRandom * rndg = new TRandom(                                  );
 TFile   * file = new TFile  ("/home/menasce/data-otsdaq/prove/aCollection.root", "recreate"    );
 
 file->mkdir("1dPlots"        );
 file->mkdir("2dPlots"        );
 file->mkdir("3dPlots"        );
 TDirectory * plots1dDir         = file->mkdir("1dPlots"        );
 TDirectory * plots2dDir         = file->mkdir("2dPlots"        );
 TDirectory * plots3dDir         = file->mkdir("3dPlots"        );
 TDirectory * TGraphsDir         = file->mkdir("TGraphs"        );
 TDirectory * TGraphs2dDir       = file->mkdir("TGraphs2d"      );
 TDirectory * TGraphs2dErrorsDir = file->mkdir("TGraphs2dErrors");
// TDirectory * THPolyDir          = file->mkdir("THPoly"         );
 
 //__________________________________________________________________________________
 file->cd("1dPlots") ;
 TH1D * h1_0 = new TH1D("h1_0","h1_0",nch, xmi, xma                               ) ;
 for(int n=0; n<10000; n++)
 {
  h1_0->Fill(rndg->Gaus(peak,width)) ;
 } 
 //__________________________________________________________________________________
 file->cd("2dPlots") ;
 TH2D * h2_0 = new TH2D("h2_0","h2_0",nch, xmi, xma, nch, xmi, xma                ) ;
 for(int n=0; n<10000; n++)
 {
  h2_0->Fill(rndg->Gaus(peak,width),rndg->Gaus(peak,width)) ;
 } 

 //__________________________________________________________________________________
 file->cd("3dPlots") ;
 TH3D * h3_0 = new TH3D("h3_0","h3_0",nch, xmi, xma, nch, xmi, xma, nch, xmi, xma ) ;
 for(int n=0; n<10000; n++)
 {
  h3_0->Fill(rndg->Gaus(peak,width),rndg->Gaus(peak,width),rndg->Gaus(peak,width) ) ;
 }  cout << __LINE__ << endl ;

 //__________________________________________________________________________________
 file->cd("TGraphs") ;
 Double_t x[100], y[100];
 Int_t n = 20;
 for (Int_t i=0;i<n;i++) 
 {
   x[i] = i*0.1;
   y[i] = 10*sin(x[i]+0.2);
 }
 TGraph* gr = new TGraph(n,x,y);
 gr->SetName("gr graph") ;
 gr->Write() ;

 //__________________________________________________________________________________
 file->cd("TGraphs2d") ;
 Double_t x2d, y2d, z2d, P = 6.;
 Int_t np = 200;
 TGraph2D *dt = new TGraph2D();
 dt->SetTitle("Graph title; X axis title; Y axis title; Z axis title");
 TRandom *r = new TRandom();
 for (Int_t N=0; N<np; N++) 
 {
    x2d = 2*P*(r->Rndm(N))-P;
    y2d = 2*P*(r->Rndm(N))-P;
    z2d = (sin(x2d)/x2d)*(sin(y2d)/y2d)+0.2;
    dt->SetPoint(N,x2d,y2d,z2d);
 }
 dt->SetName("dt graph") ;
 dt->Write() ;

 //__________________________________________________________________________________
 file->cd("TGraphs2dErrors") ;
          P = 6.;
 Double_t *rx=0, *ry=0, *rz=0;
 Double_t *ex=0, *ey=0, *ez=0;
 rx = new Double_t[np];
 ry = new Double_t[np];
 rz = new Double_t[np];
 ex = new Double_t[np];
 ey = new Double_t[np];
 ez = new Double_t[np];
 for (Int_t N=0; N<np;N++) 
 {
    rx[N] = 2*P*(r->Rndm(N))-P;
    ry[N] = 2*P*(r->Rndm(N))-P;
    rz[N] = rx[N]*rx[N]-ry[N]*ry[N];
    rx[N] = 10.+rx[N];
    ry[N] = 10.+ry[N];
    rz[N] = 40.+rz[N];
    ex[N] = r->Rndm(N);
    ey[N] = r->Rndm(N);
    ez[N] = 10*r->Rndm(N);
 }
 TGraph2DErrors *dte = new TGraph2DErrors(np, rx, ry, rz, ex, ey, ez);
 dte->SetTitle("TGraph2D with error bars: option \"ERR\"");
 dte->SetFillColor(29);
 dte->SetMarkerSize(0.8);
 dte->SetMarkerStyle(20);
 dte->SetMarkerColor(kRed);
 dte->SetLineColor(kBlue-3);
 dte->SetLineWidth(2);
 dte->SetName("dte graph") ;
 dte->Write() ;
  
 //__________________________________________________________________________________
 file->Write() ;

}
