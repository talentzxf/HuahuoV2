package online.huahuo.backend.db;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Data
@NoArgsConstructor
@AllArgsConstructor
@Table(name = "ELEMENTS")
public class ElementDB {
    @Id
    @Column(name = "id", nullable = false)
    private String elementId;

    private String fileId;
}
