# Turn-Based Strategy Game  UE5.6 C++

## Requisiti
1. Compilazione corretta, codice commentato, polimorfismo  Fatto
2. Griglia 25x25 
3. Posizionamento unita e torri 
4. AI con A 
5. Turni e condizione di vittoria 
6. Interfaccia grafica stato gioco 
7. Suggerimento range movimento 
8. Danno da contrattacco 
9. Storico mosse 
10. AI euristica ottimizzata

## Struttura Classi
- AGridCell — cella della griglia
- ABaseUnit (Abstract) — unità base, implementa IAttackable e IMovable
  - ASniper — unità a distanza (HP:20, Mov:4, Range:10, Dmg:4-8)
  - ABrawler — unità corpo a corpo (HP:40, Mov:6, Range:1, Dmg:1-6)
- ATurnBasedGameMode — gestione turni e fasi di gioco
- ATurnBasedGameState — stato condiviso della partita
- IAttackable — interfaccia attacco
- IMovable — interfaccia movimento