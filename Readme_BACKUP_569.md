## Description :  

Ce projet est une gestion d'interface web et d'affichage sur ecran TFT. Il est doté d'une interface web permettant d'envoyer une image a un ESP32-S3.  
Du côté code esp32, nous avons une gestion SPIFFS pour l'affichage de l'index et une gestion des requetes. Il y a une requete qui recois une data qui est la nouvelle image a mettre sur l'ecran.

*******  
  
## Description des fichiers :  

Parce que c'est :
 * **lib** : qui est une copies des librairies que j'ai utilisé
 * **src** : qui est le code source de mon projet
 * **tools** : qui est l'outils permettant d'envoyer des datas dans la mémoire'
 * **T-RGB** : qui est le code source du projet originel 
 * **PNG-to-h.py** : qui est le code source qui m'a permis de convertir une image PNG en .h pour faire fonctionner mon ecran  
   
*******  
  
## Comment faire fonctionner ?  

*******  

## Table des matières  :  

 1. Glissez tools dans votre dossier **Arduino**
 2. Ouvrir **lilygo_screen_wifi.ino** qui est dans **src**
 3. Allez dans **Outils** puis cliquez sur **ESP32 Sketch Data Upload** et attendez la fin du televersement
 4. Puis cliquez sur la fleches de téléversement en hauta gauche de l'IDE
     
*******  
  
## Liens utiles :  

 * Lien du produit :https://fr.aliexpress.com/item/1005005913989946.html?spm=a2g0o.order_list.order_list_main.41.18755e5bn1JnW8&gatewayAdapt=glo2fra    
 * Lien originel : https://github.com/Xinyuan-LilyGO/T-RGB?spm=a2g0o.detail.1000023.1.7251rUXsrUXsOZ

