import urllib.request 
import os
import sys
import zipfile
def download_light_field(name):
    url      = "http://lightfield.stanford.edu/data/"+name+"/"
    filename = "rectified.zip"
    def progress(block_count, block_size, total_size):
        # ''' コールバック関数 '''
        percentage = 100.0 * block_count * block_size / total_size
        # 改行したくないので print 文は使わない
        sys.stdout.write( "Downloading %s from %s: %.2f %% ( %d KB )\r" % (filename, url, percentage, total_size / 1024 ) )
    try:
        os.mkdir("Data/")
    except OSError:
        print("Data/"+ ' already exists')
    try:
        os.mkdir("Data/"+name+"/")
    except OSError:
        print("Data/"+name+"/"+ ' already exists')
    urllib.request.urlretrieve(url+filename,reporthook=progress,filename="Data/"+name+"/"+filename)
    with zipfile.ZipFile("Data/"+name+"/"+filename) as zip:
        zip.extractall("Data/"+name+"/")
def main():
    download_light_field("chess_lf")
    download_light_field("tarot_fine_lf")
main()