{
 gROOT->Reset() ;
 TFile * f = new TFile("/home/menasce/data-otsdaq/OtsHistos/Aacme.root","recreate") ;

 f->mkdir("MainDir") ;
 f->cd   ("MainDir") ;
 TH1F * h1 = new TH1F("GaussianMain1","GaussianMain1",100,0,100) ;

 f->mkdir("ParallelDir") ;
 f->cd   ("ParallelDir") ;
 TH1F * h2 = new TH1F("GaussianParallelDir2","GaussianParallelDir2",100,0,100) ;
 h2->Draw() ;
 TCanvas * c = new TCanvas("Canvas","canvas",800,800) ;
 c->Divide(2,1) ;
 TH1F * h4 = new TH1F("GaussianParallelDir4","GaussianParallelDir4",100,0,100) ;
 TH1F * h5 = new TH1F("GaussianParallelDir5","GaussianParallelDir5",100,0,100) ;
 c->cd(1) ; h4->Draw() ;
 c->cd(2) ; h5->Draw() ;
 c->Write() ;
 
 gDirectory->mkdir("ParallelSubDir") ;
 gDirectory->cd   ("ParallelSubDir") ;
 TH1F * h3 = new TH1F("GaussianParallelSubDir6","GaussianParallelSubDir6",100,0,100) ;
 h3->Draw() ;
 
 gDirectory->mkdir("ParallelSubSubDir") ;
 gDirectory->cd   ("ParallelSubSubDir") ;
 TH1F * h7 = new TH1F("GaussianParallelDir7","GaussianParallelDir7",100,0,100) ;
 h7->Draw() ;
 
 f->Write() ;
 f->Close() ;

}
