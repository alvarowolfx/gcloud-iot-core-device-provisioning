
# Primary
curl -o gtsltsr.crt https://pki.goog/gtsltsr/gtsltsr.crt
openssl x509 -inform DER -in gtsltsr.crt -out primary_ca.pem

# Backup
curl -o gsr4.crt https://pki.goog/gsr4/GSR4.crt
openssl x509 -inform DER -in gsr4.crt -out backup_ca.pem
