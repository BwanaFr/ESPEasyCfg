from datetime import datetime, date, time, timezone
import locale
import gzip
from io import BytesIO
import binascii
import shutil
import os
import mimetypes
import hashlib

locale.setlocale(locale.LC_TIME, "en_US")
dt = datetime.now()
lm = dt.strftime("%a, %d %b %Y %H:%M:%S GMT")

print('Last-modified : ', lm)
totalBytes = 0
files = []
with open('./src/StaticContent.cpp', 'w') as out_f:
    out_f.write('#include "StaticContent.h"\n\n')
    out_f.write('//Automatically generated with make_content script, do not edit!\n\n')
    for filename in os.listdir('./data'):
        absPath = './data/' + filename
        if os.path.isfile(absPath):
            print('Compressing file ', absPath)
            buf = BytesIO()
            with open(absPath, 'rb') as f_in:
                with gzip.GzipFile(mode='wb', fileobj=buf) as f_out:
                    shutil.copyfileobj(f_in, f_out)
            compressed = buf.getvalue()
            print('Compressed Data:', len(compressed), ' bytes')
            totalBytes += len(compressed)
            mimetype = mimetypes.guess_type(filename)[0]
            print('mimetype : ', mimetype)
            etag = hashlib.sha1(compressed).hexdigest()
            files.append(filename)
            f_n = filename.replace('.', "_").replace('-', '_')
            out_f.write(f'static const size_t {f_n}_len PROGMEM = {len(compressed)};\n')
            out_f.write(f'static const char* {f_n}_mimetype PROGMEM = "{mimetype}";\n')
            out_f.write(f'static const char* {f_n}_etag PROGMEM = "{etag}";\n')
            out_f.write(f'const uint8_t {f_n}_data[] PROGMEM = {{')
            for b in compressed:
                hex_data = "0x{0:x},".format(b)
                out_f.write(hex_data)
            out_f.write('};\n\n')
    out_f.write(f'static const char* static_files_last_modified PROGMEM = "{lm}";\n')
    out_f.write(f'static const char* cache_control_header PROGMEM = "public, max-age=31536000";\n')
    out_f.write('AsyncWebHandler* registerStaticFiles(AsyncWebServer* webServer){\n')
    out_f.write('\tAsyncWebHandler* ret = nullptr;\n')
    for f in files:
        f_n = f.replace('.', "_").replace('-', '_')
        out_f.write('\t')
        if(f == 'config.html'):
            out_f.write('ret = &')
        out_f.write(f'webServer->on("/www/{f}", HTTP_GET, [=](AsyncWebServerRequest *request){{\n')
        out_f.write('\t\tif (request->header("If-Modified-Since") == static_files_last_modified) {\n')
        out_f.write('\t\t\trequest->send(304);\n')
        out_f.write(f'\t\t}}else if (request->hasHeader("If-None-Match") && request->header("If-None-Match").equals({f_n}_etag)) {{\n')
        out_f.write('\t\t\tAsyncWebServerResponse * response = new AsyncBasicResponse(304); // Not modified\n')
        out_f.write('\t\t\tresponse->addHeader("Cache-Control", cache_control_header);\n')
        out_f.write(f'\t\t\tresponse->addHeader("ETag", {f_n}_etag);\n')
        out_f.write('\t\t\trequest->send(response);\n')
        out_f.write('\t\t}else{\n')
        out_f.write(f'\t\t\tAsyncWebServerResponse *response = request->beginResponse_P(200, {f_n}_mimetype, {f_n}_data, {f_n}_len);\n')
        out_f.write('\t\t\tresponse->addHeader("Content-Encoding", "gzip");\n')
        out_f.write('\t\t\tresponse->addHeader("Cache-Control", cache_control_header);\n')
        out_f.write(f'\t\t\tresponse->addHeader("ETag", {f_n}_etag);\n')
        out_f.write('\t\t\trequest->send(response);\n')
        out_f.write('\t\t}\n')
        out_f.write('\t});\n\n')
        
    out_f.write('\treturn ret;\n}\n')

print('Content total bytes : ', totalBytes)
