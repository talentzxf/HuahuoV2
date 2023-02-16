class ScoreManager{
    scoreSpan
    lifeSpan
    score = 0
    life = 5
    constructor() {
        this.scoreSpan = document.createElement("div")
        this.lifeSpan = document.createElement("div")

        this.scoreSpan.innerText = "Current Score:" + this.score
        this.lifeSpan.innerText = "Current lifes:" + this.life

        document.body.append(this.scoreSpan)
        document.body.append(this.lifeSpan)
    }

    deduceLife(){
        this.life--
        this.lifeSpan.innerText = "Current lifes:" + this.life
    }

    addScore(){
        this.score++
        this.scoreSpan.innerText = "Current Score:" + this.score
    }
}
let scoreManager = new ScoreManager()
export {scoreManager}