{
  TFile* file = TFile::Open("streamerInfo.root", "RECREATE");
  file->mkdir("Directory","Directory");
  file->cd("Directory");
  TH1F*     h1f = new TH1F    ("hTH1F","TH1F",1,0,1);
  TH2F*     h2f = new TH2F    ("hTH2F","TH2F",1,0,1,1,0,1);
  TH1I*     h1I = new TH1I    ("hTH1I","TH1I",1,0,1);
  TProfile* hp  = new TProfile("hProfile","TProfile",1,0,1);
  TCanvas*  c   = new TCanvas ("TCanvas","TCanvas");
  c->Write();
  file->Write();
  
}
