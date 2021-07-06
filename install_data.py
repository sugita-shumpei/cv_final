import urllib.request 
import os;
import sys
def download_light_field():
    url      = "http://lightfield.stanford.edu/data/bulldozer_lf/"
    filename = "rectified.zip"
    def progress(block_count, block_size, total_size):
        # ''' コールバック関数 '''
        percentage = 100.0 * block_count * block_size / total_size
        # 改行したくないので print 文は使わない
        sys.stdout.write( "Downloading %s from %s: %.2f %% ( %d KB )\r" % (filename, url, percentage, total_size / 1024 ) )
    
    os.makedirs("Data/bulldozer_lf/")
    urllib.request.urlretrieve(url+filename,reporthook=progress,filename="Data/bulldozer_lf/"+filename)
def main():
    download_light_field()

main()