TP chargement asynchrone

Un code asynchrone se caractérise par deux aspects : 
-	Une exécution concurrente (parallèle)
-	Une absence ou faible usage de la synchronisation

On souhaite être capable de charger une image depuis le disque sans pour autant bloquer le thread principal.

Pour ce faire, on créé un thread -qui peut éventuellement être « détaché »- qui va exécuter le code de chargement des images.
En C++11 on peut utiliser un std::thread ou alors std::async si l’on préfère voir notre loader comme une tâche de fond. 

note : dans l’état actuel du C++, std ::async créé quasiment toujours un nouveau thread, sans garantie d’un thread pool en interne. On a tout de même deux polices à spécifier explicitement : std::launch::deferred si vous ne souhaitez pas créer plus de thread que de cœurs disponibles sur le système, ou à contrario std::launch::async … mais n’utilisez jamais la combinaison des deux (qui est l’option par défaut sur certains compilateurs) qui peut entraîner des bugs de synchronisation.

Bien que tout (chargement, décompression, allocation) se fasse de manière concurrente, il faut néanmoins un peu de communication entre les deux threads.

Tout d’abord il est souvent utile d’être informé lorsqu’un thread est prêt à exécuter sa tâche (après une phase de setup par exemple). De même il est pratique de pouvoir attendre la fin d’une tâche. Ce dernier point peut être facilement mis en place via un join() -si le thread n’est pas détaché- ou un get() si std::async. 

Ce dernier point est assez important car une tâche concurrente risque de continuer à traiter des données, ou modifier des états qui seraient invalides par suite du fait que le thread principal à enclenché une procédure de libération de la mémoire. Exemple : un std::vector a été détruit et un thread essaye de faire un push_back(), cela conduit nécessairement à une exception ou crash).

Note : detach() sur un thread peut aider à libérer les ressources propres du thread, car il est non joignable et donc async, cependant cela ne pourra résoudre le type de problèmes décrit ci-dessus.

Le thread principal doit informer le thread loader afin de lui indiquer qu’il y’a du travail à effectuer (chemin de l’image, et éventuellement d’autres métadonnées).
Ceci va d’ailleurs nous permettre de réduire l’occupation CPU du thread loader lorsqu’il n’est pas mis à contribution.
Le thread loader doit quant à lui informer le thread principal de la validité ou non des données : on récupère par exemple des métadonnées (dimensions…) ainsi qu’un pointeur (null indiquant une erreur).
